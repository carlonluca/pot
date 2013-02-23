/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    11.01.2012
 *
 * Copyright (c) 2012, 2013 Luca Carlon. All rights reserved.
 *
 * This file is part of PiOmxTextures.
 *
 * PiOmxTextures is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PiOmxTextures is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures.  If not, see <http://www.gnu.org/licenses/>.
 */

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QStringList>

#include <cstring>

#include "lgl_logging.h"
#include "omx_mediaprocessor.h"

#define ENABLE_HDMI_CLOCK_SYNC false
#define ENABLE_SUBTITLES       false
#define FONT_PATH              "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
#define FONT_SIZE              (0.055f)
#define CENTERED               false
#define BUFFERING_TIMEOUT_S    3


/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::OMX_MediaProcessor
+-----------------------------------------------------------------------------*/
OMX_MediaProcessor::OMX_MediaProcessor(OMX_TextureProvider* provider) :
    m_state(STATE_INACTIVE),
    m_omx_pkt(NULL),
    m_bMpeg(false),
    m_has_video(false),
    m_has_audio(false),
    m_has_subtitle(false),
    m_buffer_empty(true),
    m_subtitle_index(0),
    m_audio_index(0),
    m_provider(provider)
{
    m_RBP.Initialize();
    m_OMX.Initialize();

    // Players.
    m_av_clock         = new OMXClock;
    m_player_video     = new OMXPlayerVideo(provider);
    m_player_audio     = new OMXPlayerAudio;
    m_player_subtitles = new OMXPlayerSubtitles;

    // Move to a new thread.
    moveToThread(&m_thread);
    m_thread.start();
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::~OMX_MediaProcessor
+-----------------------------------------------------------------------------*/
OMX_MediaProcessor::~OMX_MediaProcessor()
{
    m_OMX.Deinitialize();
    m_RBP.Deinitialize();

    delete m_av_clock;
    delete m_player_subtitles;
    delete m_player_audio;
    delete m_player_video;

    m_thread.quit();
    m_thread.wait();
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::filename
+-----------------------------------------------------------------------------*/
QString OMX_MediaProcessor::filename()
{
    return m_filename;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::streams
+-----------------------------------------------------------------------------*/
QStringList OMX_MediaProcessor::streams()
{
    // TODO: Reimplement.
    return QStringList();
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::setFilename
+-----------------------------------------------------------------------------*/
bool OMX_MediaProcessor::setFilename(QString filename, GLuint& textureId)
{
    QMutexLocker locker(&m_sendCmd);
    if (!checkCurrentThread())
        return false;

    switch (m_state) {
    case STATE_INACTIVE:
        break;
    case STATE_PAUSED:
    case STATE_PLAYING:
    case STATE_STOPPED:
        return false; // TODO: Reimplement.
    }

    m_filename = filename;

    LOG_VERBOSE(LOG_TAG, "Opening video file...");
    if (!m_omx_reader.Open(m_filename.toStdString(), true))
        return false;

    m_bMpeg         = m_omx_reader.IsMpegVideo();
    m_has_video     = m_omx_reader.VideoStreamCount();
    m_has_audio     = m_omx_reader.AudioStreamCount();
    m_has_subtitle  = m_omx_reader.SubtitleStreamCount();

    LOG_VERBOSE(LOG_TAG, "Initializing OMX clock...");
    if (!m_av_clock->OMXInitialize(m_has_video, m_has_audio))
        return false;

    if (ENABLE_HDMI_CLOCK_SYNC && !m_av_clock->HDMIClockSync())
        return false;

    m_omx_reader.GetHints(OMXSTREAM_AUDIO, m_hints_audio);
    m_omx_reader.GetHints(OMXSTREAM_VIDEO, m_hints_video);

    // Set audio stream to use.
    // TODO: Implement a way to change it runtime.
#if 0
    m_omx_reader.SetActiveStream(OMXSTREAM_AUDIO, m_audio_index_use);
#endif

    // Seek on start?
#if 0
    if (m_seek_pos !=0 && m_omx_reader.CanSeek()) {
        printf("Seeking start of video to %i seconds\n", m_seek_pos);
        m_omx_reader.SeekTime(m_seek_pos * 1000.0f, 0, &startpts);  // from seconds to DVD_TIME_BASE
    }
#endif

    LOG_VERBOSE(LOG_TAG, "Opening video using OMX...");
    if (m_has_video)
        if (!m_player_video->Open(
                    m_hints_video,
                    m_av_clock,
                    textureId,
                    false,                  /* deinterlace */
                    m_bMpeg,
                    ENABLE_HDMI_CLOCK_SYNC,
                    true,                   /* threaded */
                    1.0                     /* display aspect, unused */
                    ))
            return false;

    LOG_VERBOSE(LOG_TAG, "Opening subtitles using OMX...");
    if (m_has_subtitle)
        if (!m_player_subtitles->Open(FONT_PATH, FONT_SIZE, CENTERED, m_av_clock))
            return false;

    // This is an upper bound check on the subtitle limits. When we pulled the subtitle
    // index from the user we check to make sure that the value is larger than zero, but
    // we couldn't know without scanning the file if it was too high. If this is the case
    // then we replace the subtitle index with the maximum value possible.
    if (m_has_subtitle && m_subtitle_index > (m_omx_reader.SubtitleStreamCount() - 1))
        m_subtitle_index = m_omx_reader.SubtitleStreamCount() - 1;

    m_omx_reader.GetHints(OMXSTREAM_AUDIO, m_hints_audio);

    LOG_VERBOSE(LOG_TAG, "Opening audio using OMX...");
    if (m_has_audio)
        if (!m_player_audio->Open(
                    m_hints_audio,
                    m_av_clock,
                    &m_omx_reader,
                    "omx:hdmi",         /* TODO: implement way to change */
                    false,              /* TODO: passthrough */
                    false,              /* TODO: hw decode */
                    false,              /* TODO: downmix boost */
                    true                /* threaded */
                    ))
            return false;

    LOG_VERBOSE(LOG_TAG, "Executing clock...");
    m_av_clock->SetSpeed(DVD_PLAYSPEED_NORMAL); /* TODO: Implement speed */
    m_av_clock->OMXStateExecute();

    m_state = STATE_STOPPED;
    return true;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::play
+-----------------------------------------------------------------------------*/
bool OMX_MediaProcessor::play()
{
    // I need to invoke this in another thread (this object is owned by another
    // thread).
    LOG_VERBOSE(LOG_TAG, "Play");
    QMutexLocker locker(&m_sendCmd);
    if (!checkCurrentThread())
        return false;

    switch (m_state) {
    case STATE_INACTIVE:
        return false;
    case STATE_PAUSED:
        m_state = STATE_PLAYING;
        setSpeed(OMX_PLAYSPEED_NORMAL);
        m_av_clock->OMXResume();
        return true;
    case STATE_PLAYING:
        return true;
    case STATE_STOPPED: {
        LOG_VERBOSE(LOG_TAG, "Starting thread.");
        m_state = STATE_PLAYING;
        m_av_clock->OMXStart();
        return QMetaObject::invokeMethod(this, "mediaDecoding");
    }
    default:
        return false;
    }
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::stop
+-----------------------------------------------------------------------------*/
bool OMX_MediaProcessor::stop()
{
    LOG_VERBOSE(LOG_TAG, "Stop");
    QMutexLocker locker(&m_sendCmd);
    if (!checkCurrentThread())
        return false;

    // TODO: I should wait for success.
    switch (m_state) {
    case STATE_INACTIVE:
        return false;
    case STATE_PAUSED:
    case STATE_PLAYING:
    case STATE_STOPPED:
        m_state = STATE_STOPPED;
        return true;
    default:
        return false;
    }
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::pause
+-----------------------------------------------------------------------------*/
bool OMX_MediaProcessor::pause()
{
    LOG_VERBOSE(LOG_TAG, "Pause");
    QMutexLocker locker(&m_sendCmd);
    if (!checkCurrentThread())
        return false;

    switch (m_state) {
    case STATE_INACTIVE:
    case STATE_STOPPED:
        return false;
    case STATE_PAUSED:
    case STATE_PLAYING:
        m_state = STATE_PAUSED;
        setSpeed(OMX_PLAYSPEED_PAUSE);
        m_av_clock->OMXPause();
        return true;
    default:
        return false;
    }
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::seek
+-----------------------------------------------------------------------------*/
bool OMX_MediaProcessor::seek(long /* position */)
{
    return false;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::currentPosition
+-----------------------------------------------------------------------------*/
long OMX_MediaProcessor::currentPosition()
{
    return false;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::textureId
+-----------------------------------------------------------------------------*/
GLuint OMX_MediaProcessor::textureId()
{
    return m_textureId;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::mediaDecoding
+-----------------------------------------------------------------------------*/
void OMX_MediaProcessor::mediaDecoding()
{
    LOG_VERBOSE(LOG_TAG, "Decoding thread started.");
    emit playbackStarted();

    struct timespec starttime, endtime;
    while (m_state != STATE_STOPPED) {
        // TODO: Use a semaphore instead.
        if (m_state == STATE_PAUSED) {
            OMXClock::OMXSleep(2);
            continue;
        }

#if 0 // TODO: Reimplement?
        if (m_incr != 0 && !m_bMpeg) {
            int    seek_flags   = 0;
            double seek_pos     = 0;
            double pts          = 0;

            pts = m_av_clock->GetPTS();

            seek_pos = (pts / DVD_TIME_BASE) + m_incr;
            seek_flags = m_incr < 0.0f ? AVSEEK_FLAG_BACKWARD : 0;

            seek_pos *= 1000.0f;

            m_incr = 0;

            if(m_omx_reader.SeekTime(seek_pos, seek_flags, &startpts))
                FlushStreams(startpts);

            m_player_video->Close();
            if(m_has_video && !m_player_video->Open(m_hints_video, m_av_clock, m_Deinterlace,  m_bMpeg,
                                                    m_hdmi_clock_sync, m_thread_player, m_display_aspect))
                goto do_exit;
        }
#endif

        // TODO: Better error handling.
        if (m_player_audio->Error()) {
            LOG_ERROR(LOG_TAG, "Audio player error. emergency exit!");
            break;
        }

        if (false) {
            LOG_INFORMATION(LOG_TAG, "V : %8.02f %8d %8d A : %8.02f %8.02f Cv : %8d Ca : %8d",
                   m_av_clock->OMXMediaTime(), m_player_video->GetDecoderBufferSize(),
                   m_player_video->GetDecoderFreeSpace(), m_player_audio->GetCurrentPTS() / DVD_TIME_BASE,
                   m_player_audio->GetDelay(), m_player_video->GetCached(), m_player_audio->GetCached());
        }

        if (m_omx_reader.IsEof() && !m_omx_pkt) {
            if (!m_player_audio->GetCached() && !m_player_video->GetCached())
                break;

            // Abort audio buffering, now we're on our own.
            if (m_buffer_empty)
                m_av_clock->OMXResume();

            OMXClock::OMXSleep(10);
            continue;
        }

        /* when the audio buffer runs under 0.1 seconds we buffer up */
        if (m_has_audio) {
            if (m_player_audio->GetDelay() < 0.1f && !m_buffer_empty) {
                if (!m_av_clock->OMXIsPaused()) {
                    m_av_clock->OMXPause();

                    LOG_VERBOSE(LOG_TAG, "Buffering starts.");
                    m_buffer_empty = true;
                    clock_gettime(CLOCK_REALTIME, &starttime);
                }
            }
            if (m_player_audio->GetDelay() > (AUDIO_BUFFER_SECONDS * 0.75f) && m_buffer_empty) {
                if (m_av_clock->OMXIsPaused()) {
                    m_av_clock->OMXResume();

                    LOG_VERBOSE(LOG_TAG, "Buffering ends.");
                    m_buffer_empty = false;
                }
            }

            if (m_buffer_empty) {
                clock_gettime(CLOCK_REALTIME, &endtime);
                if ((endtime.tv_sec - starttime.tv_sec) > BUFFERING_TIMEOUT_S) {
                    m_buffer_empty = false;
                    m_av_clock->OMXResume();
                    LOG_WARNING(LOG_TAG, "Buffering timed out.");
                }
            }
        }

        if (!m_omx_pkt)
            m_omx_pkt = m_omx_reader.Read();

        if (m_has_video && m_omx_pkt && m_omx_reader.IsActive(OMXSTREAM_VIDEO, m_omx_pkt->stream_index)) {
            if (m_player_video->AddPacket(m_omx_pkt))
                m_omx_pkt = NULL;
            else
                OMXClock::OMXSleep(10);

#if 0 // TODO: Reimplement?
            if(m_tv_show_info)
            {
                char response[80];
                vc_gencmd(response, sizeof response, "render_bar 4 video_fifo %d %d %d %d",
                          m_player_video->GetDecoderBufferSize()-m_player_video->GetDecoderFreeSpace(),
                          0 , 0, m_player_video->GetDecoderBufferSize());
                vc_gencmd(response, sizeof response, "render_bar 5 audio_fifo %d %d %d %d",
                          (int)(100.0*m_player_audio->GetDelay()), 0, 0, 100*AUDIO_BUFFER_SECONDS);
            }
#endif
        }
        else if (m_has_audio && m_omx_pkt && m_omx_pkt->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (m_player_audio->AddPacket(m_omx_pkt))
                m_omx_pkt = NULL;
            else
                OMXClock::OMXSleep(10);
        }
        else if (m_omx_pkt && m_omx_reader.IsActive(OMXSTREAM_SUBTITLE, m_omx_pkt->stream_index)) {
            if (m_omx_pkt->size && ENABLE_SUBTITLES &&
                    (m_omx_pkt->hints.codec == CODEC_ID_TEXT ||
                     m_omx_pkt->hints.codec == CODEC_ID_SSA)) {
                if(m_player_subtitles->AddPacket(m_omx_pkt))
                    m_omx_pkt = NULL;
                else
                    OMXClock::OMXSleep(10);
            }
            else {
                m_omx_reader.FreePacket(m_omx_pkt);
                m_omx_pkt = NULL;
            }
        }
        else {
            if (m_omx_pkt) {
                m_omx_reader.FreePacket(m_omx_pkt);
                m_omx_pkt = NULL;
            }
        }
    }

    emit playbackCompleted();
    cleanup();
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::setSpeed
+-----------------------------------------------------------------------------*/
inline
void OMX_MediaProcessor::setSpeed(int iSpeed)
{
    if (!m_av_clock)
        return;

    if (iSpeed < OMX_PLAYSPEED_PAUSE)
        return;

    m_omx_reader.SetSpeed(iSpeed);

    if (m_av_clock->OMXPlaySpeed() != OMX_PLAYSPEED_PAUSE && iSpeed == OMX_PLAYSPEED_PAUSE)
        m_state = STATE_PAUSED;
    else if (m_av_clock->OMXPlaySpeed() == OMX_PLAYSPEED_PAUSE && iSpeed != OMX_PLAYSPEED_PAUSE)
        m_state = STATE_PLAYING;

    m_av_clock->OMXSpeed(iSpeed);
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::checkCurrentThread
+-----------------------------------------------------------------------------*/
inline
bool OMX_MediaProcessor::checkCurrentThread()
{
    if (QThread::currentThreadId() == m_thread.getThreadId()) {
        LOG_ERROR(LOG_TAG, "Do not invoke in the object's thread!");
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::cleanup
+-----------------------------------------------------------------------------*/
inline
void OMX_MediaProcessor::cleanup()
{
    LOG_INFORMATION(LOG_TAG, "Cleaning up...");

    QMutexLocker locker(&m_sendCmd);
    if (m_state != STATE_STOPPED /* && !g_abort */) {
        if (m_has_audio)
            m_player_audio->WaitCompletion();
        else if (m_has_video)
            m_player_video->WaitCompletion();
    }

#if 0
    if (m_refresh)
    {
        m_BcmHost.vc_tv_hdmi_power_on_best(
                    tv_state.width,
                    tv_state.height,
                    tv_state.frame_rate,
                    HDMI_NONINTERLACED,
                    (EDID_MODE_MATCH_FLAG_T)(HDMI_MODE_MATCH_FRAMERATE|
                                             HDMI_MODE_MATCH_RESOLUTION|HDMI_MODE_MATCH_SCANMODE)
                    );
    }
#endif

    m_av_clock->OMXStop();
    m_av_clock->OMXStateIdle();

    m_player_subtitles->Close();
    m_player_video->Close();
    m_player_audio->Close();

    if (m_omx_pkt) {
        m_omx_reader.FreePacket(m_omx_pkt);
        m_omx_pkt = NULL;
    }

    m_omx_reader.Close();

    vc_tv_show_info(0);

    m_OMX.Deinitialize();
    m_RBP.Deinitialize();

    // lcarlon: free the texture.
    QMetaObject::invokeMethod(
                (QObject*)m_provider,
                "freeTexture",
                Qt::DirectConnection,
                Q_ARG(GLuint, m_textureId)
                );
}
