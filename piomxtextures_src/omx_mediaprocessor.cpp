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
#include <QOpenGLContext>
#include <QElapsedTimer>
#include <QUrl>

#include <cstring>

#include "lc_logging.h"

#include "omx_mediaprocessor.h"
#include "omx_textureprovider.h"
#include "omx_playeraudio.h"
#include "omx_reader.h"
#include "omx_utils.h"

// omxplayer lib.
#include "omxplayer_lib/linux/RBP.h"
#include "omxplayer_lib/OMXPlayerVideo.h"
#include "omxplayer_lib/OMXPlayerAudio.h"
#include "omxplayer_lib/OMXPlayerSubtitles.h"
#include "omxplayer_lib/OMXStreamInfo.h"
#include "omxplayer_lib/DllOMX.h"
#include "omxplayer_lib/OMXVideo.h"
#include "omxplayer_lib/OMXAudio.h"

#define ENABLE_HDMI_CLOCK_SYNC false
#define FONT_PATH              "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
#define FONT_PATH_ITALIC       "/usr/share/fonts/truetype/freefont/FreeSansOblique.ttf"
#define FONT_SIZE              (0.055f)
#define CENTERED               false
#define BUFFERING_TIMEOUT_S    3

// when we repeatedly seek, rather than play continuously
#define TRICKPLAY(speed) (speed < 0 || speed > 4 * DVD_PLAYSPEED_NORMAL)

#define S(x) (int)(DVD_PLAYSPEED_NORMAL*(x))
int playspeeds[] = {
   S(0), S(1/16.0), S(1/8.0), S(1/4.0), S(1/2.0),
   S(0.975), S(1.0), S(1.125), S(-32.0), S(-16.0),
   S(-8.0), S(-4), S(-2), S(-1), S(1), S(2.0), S(4.0),
   S(8.0), S(16.0), S(32.0)
};
const int playspeed_slow_min = 0,
playspeed_slow_max = 7, playspeed_rew_max = 8,
playspeed_rew_min = 13, playspeed_normal = 14,
playspeed_ff_min = 15, playspeed_ff_max = 19;

/*------------------------------------------------------------------------------
|    get_mem_gpu
+-----------------------------------------------------------------------------*/
static int get_mem_gpu(void)
{
   char response[80] = "";
   int gpu_mem = 0;
   if (vc_gencmd(response, sizeof response, "get_mem gpu") == 0)
      vc_gencmd_number_property(response, "gpu", &gpu_mem);
   return gpu_mem;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::OMX_MediaProcessor
+-----------------------------------------------------------------------------*/
OMX_MediaProcessor::OMX_MediaProcessor(OMX_EGLBufferProviderSh provider) :
   m_provider(provider),
   m_state(STATE_INACTIVE),
#ifdef ENABLE_SUBTITLES
   m_has_subtitle(false),
#endif
	m_av_clock(new OMXClock),
	m_player_video(new OMXPlayerVideo(provider)),
	m_player_audio(new OMX_PlayerAudio),
	m_omx_reader(new OMX_Reader),
   m_omx_pkt(NULL),
   m_RBP(new CRBP),
   m_OMX(new COMXCore),
   m_has_video(false),
   m_has_audio(false),
   m_buffer_empty(true),
   m_pendingStop(false),
   m_pendingPause(false),
   m_subtitle_index(0),
   m_audio_index(0),
   m_incr(0),
   m_audioConfig(new OMXAudioConfig()),
   m_videoConfig(new OMXVideoConfig()),
   m_playspeedCurrent(playspeed_normal),
   m_seekFlush(false),
   m_packetAfterSeek(false),
   startpts(0),
   m_loop(false),
	m_muted(false),
   m_loop_from(0.0f),
   m_fps(0.0f)
{
   qRegisterMetaType<OMX_MediaProcessor::OMX_MediaProcessorError>("OMX_MediaProcessor::OMX_MediaProcessorError");
   qRegisterMetaType<OMX_MediaProcessor::OMX_MediaProcessorState>("OMX_MediaProcessor::OMX_MediaProcessorState");

   int gpu_mem = get_mem_gpu();
   int min_gpu_mem = 64;
   if (gpu_mem > 0 && gpu_mem < min_gpu_mem)
      LOG_WARNING(LOG_TAG, "Only %dM of gpu_mem is configured. Try running"
                  " \"sudo raspi-config\" and ensure that \"memory_split\" has "
                  "a value of %d or greater\n", gpu_mem, min_gpu_mem);

   m_RBP->Initialize();
   m_OMX->Initialize();

   // Move to a new thread.
   moveToThread(&m_thread);
   m_thread.start();
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::~OMX_MediaProcessor
+-----------------------------------------------------------------------------*/
OMX_MediaProcessor::~OMX_MediaProcessor()
{
   stop();

   LOG_VERBOSE(LOG_TAG, "Waiting for thread to die...");
   m_thread.quit();
   m_thread.wait();

   // TODO: Fix this! When freeing players, a lock seems to hang!
   LOG_VERBOSE(LOG_TAG, "Freeing players...");
#ifdef ENABLE_SUBTITLES
   delete m_player_subtitles;
#endif
   delete m_player_audio;
   delete m_player_video;

   LOG_VERBOSE(LOG_TAG, "Freeing clock...");
   if (m_av_clock) {
      m_av_clock->OMXDeinitialize();
      delete m_av_clock;
   }

   // TODO: This should really be done, but still it seems to sefault sometimes.
   LOG_VERBOSE(LOG_TAG, "Deinitializing hardware libs...");
   m_OMX->Deinitialize();
   m_RBP->Deinitialize();

   LOG_VERBOSE(LOG_TAG, "Freeing hints...");
   delete m_audioConfig;
   delete m_videoConfig;

   LOG_VERBOSE(LOG_TAG, "Freeing OpenMAX structures...");
   delete m_RBP;
   delete m_OMX;
   delete m_omx_pkt;
   delete m_omx_reader;
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
/**
 * @brief OMX_MediaProcessor::setFilename Sets the filename and returns the texture
 * data that will be used.
 * @param filename
 * @param textureData
 * @return
 */
bool OMX_MediaProcessor::setFilename(QString filename, OMX_TextureData*& textureData)
{
   QMutexLocker locker(&m_sendCmd);
   if (!checkCurrentThread())
      return false;
   return setFilenameInt(filename, textureData);
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::setFilenameInt
+-----------------------------------------------------------------------------*/
inline
bool OMX_MediaProcessor::setFilenameInt(QString filename, OMX_TextureData*& textureData)
{
   log_verbose_func;

   switch (m_state) {
   case STATE_INACTIVE:
      break;
   case STATE_STOPPED:
      cleanup();
      break;
   case STATE_PAUSED:
   case STATE_PLAYING:
      return false; // TODO: Reimplement.
   }

   LOG_VERBOSE(LOG_TAG, "Opening %s...", qPrintable(filename));

   // It seems that omxplayer expects path and not local URIs.
   QUrl url(filename);
   if (url.isLocalFile() && filename.startsWith("file://"))
      filename = url.path();

	//m_omx_reader = new OMX_Reader;
   if (!m_omx_reader->Open(filename.toStdString(), true)) {
      log_err("Failed to open source %s.", qPrintable(filename));
      return false;
   }

   // Emit a signal transmitting the matadata. Anyway, I'm not sure where to check for
   // metadata actually changed or not... I emit the signal anyway, then the receiver
   // decides.
   LOG_VERBOSE(LOG_TAG, "Copy metatada...");
   convertMetaData();
   emit metadataChanged(m_metadata);

   // Players.
	//m_av_clock         = new OMXClock;
	//m_player_video     = new OMXPlayerVideo(m_provider);
	//m_player_audio     = new OMX_PlayerAudio;
//#ifdef ENABLE_SUBTITLES
//   m_player_subtitles = new OMXPlayerSubtitles;
//#endif

	// Set the mute property according to current state.
	m_player_audio->SetMuted(m_muted);

   m_filename = filename;

   m_has_video     = m_omx_reader->VideoStreamCount();
   m_has_audio     = m_omx_reader->AudioStreamCount();
#ifdef ENABLE_SUBTITLES
   m_has_subtitle  = m_omx_reader->SubtitleStreamCount();
#endif
   m_loop          = m_loop && m_omx_reader->CanSeek();

   LOG_VERBOSE(LOG_TAG, "Initializing OMX clock...");
   if (!m_av_clock->OMXInitialize())
      return false;

   if (ENABLE_HDMI_CLOCK_SYNC && !m_av_clock->HDMIClockSync())
      return false;

   m_av_clock->OMXStateIdle();
   m_av_clock->OMXStop();
   m_av_clock->OMXPause();

   m_omx_reader->GetHints(OMXSTREAM_AUDIO, m_audioConfig->hints);
   m_omx_reader->GetHints(OMXSTREAM_VIDEO, m_videoConfig->hints);

   if (m_fps > 0.0f)
       m_videoConfig->hints.fpsrate = m_fps * DVD_TIME_BASE, m_videoConfig->hints.fpsscale = DVD_TIME_BASE;

   // Set audio stream to use.
   // TODO: Implement a way to change it runtime.
#if 0
   m_omx_reader->SetActiveStream(OMXSTREAM_AUDIO, m_audio_index_use);
#endif

   // Seek on start?
#if 0
   if (m_seek_pos !=0 && m_omx_reader->CanSeek()) {
      printf("Seeking start of video to %i seconds\n", m_seek_pos);
      m_omx_reader->SeekTime(m_seek_pos * 1000.0f, false, &startpts);  // from seconds to DVD_TIME_BASE
   }
#endif

   if (m_has_video) {
      LOG_VERBOSE(LOG_TAG, "Opening video using OMX...");
      if (!m_player_video->Open(m_av_clock, *m_videoConfig))
         return false;
   }

#ifdef ENABLE_SUBTITLES
   LOG_VERBOSE(LOG_TAG, "Opening subtitles using OMX...");
   if(m_has_subtitle &&
         !m_player_subtitles.Open(m_omx_reader.SubtitleStreamCount(),
                                  std::move(external_subtitles),
                                  m_font_path,
                                  m_italic_font_path,
                                  m_font_size,
                                  m_centered,
                                  m_subtitle_lines,
                                  m_av_clock))
      return false;

   // This is an upper bound check on the subtitle limits. When we pulled the subtitle
   // index from the user we check to make sure that the value is larger than zero, but
   // we couldn't know without scanning the file if it was too high. If this is the case
   // then we replace the subtitle index with the maximum value possible.
   if (m_has_subtitle && m_subtitle_index > (m_omx_reader->SubtitleStreamCount() - 1))
      m_subtitle_index = m_omx_reader->SubtitleStreamCount() - 1;
#endif

   m_omx_reader->GetHints(OMXSTREAM_AUDIO, m_audioConfig->hints);

#if 0
   if ((m_hints_audio.codec == CODEC_ID_AC3 || m_hints_audio.codec == CODEC_ID_EAC3) &&
       m_BcmHost.vc_tv_hdmi_audio_supported(EDID_AudioFormat_eAC3, 2, EDID_AudioSampleRate_e44KHz, EDID_AudioSampleSize_16bit ) != 0)
      m_passthrough = false;
   if (m_hints_audio.codec == CODEC_ID_DTS &&
       m_BcmHost.vc_tv_hdmi_audio_supported(EDID_AudioFormat_eDTS, 2, EDID_AudioSampleRate_e44KHz, EDID_AudioSampleSize_16bit ) != 0)
      m_passthrough = false;
#endif

   // Read the output location.
   QByteArray a = qgetenv("AUDIO_OUT");
   if (a.isNull())
      m_audioConfig->device = "omx:hdmi";
   else if (a == QByteArray("hdmi"))
      m_audioConfig->device = "omx:hdmi";
   else if (a == QByteArray("local"))
      m_audioConfig->device = "omx:local";
   else if (a == QByteArray("both"))
      m_audioConfig->device = "omx:both";
   else
      m_audioConfig->device = "omx:hdmi";

   log_verbose("Opening audio using OMX...");
   log_verbose("Using %s output device...", m_audioConfig->device.c_str());
   if (m_has_audio) {
      if (!m_player_audio->Open(m_av_clock, *m_audioConfig, m_omx_reader))
         return false;
      if (m_has_audio)
          m_player_audio->SetVolume(pow(10, 0 / 2000.0));
   }

   setState(STATE_STOPPED);
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

   LOG_VERBOSE(LOG_TAG, "Cleaning textures...");
   m_provider->cleanTextures();

   switch (m_state) {
   case STATE_INACTIVE:
      return false;
   case STATE_PAUSED:
      break;
   case STATE_PLAYING:
      return true;
   case STATE_STOPPED: {
      OMX_TextureData* d = NULL;
		if (!setFilenameInt(m_filename, d))
			return false;

      setState(STATE_PLAYING);

#if 0
      if (!m_omx_reader->SeekTime(0, true, &startpts)) {
         LOG_WARNING(LOG_TAG, "Failed to seek to the beginning.");
         return false;
      }

      // This can't be done here otherwise artifacts are shown at the beginning
      // of the video.
      //flushStreams(startpts);
#endif

      //m_av_clock->OMXStart(0.0);
      m_av_clock->OMXPause();
      m_av_clock->OMXStateExecute();
      m_av_clock->OMXResume();
      //m_av_clock->OMXMediaTime(0.0D);

      LOG_VERBOSE(LOG_TAG, "Starting thread.");
      return QMetaObject::invokeMethod(this, "mediaDecoding");
   }
   default:
      return false;
   }

   setState(STATE_PLAYING);
   if (m_av_clock->OMXPlaySpeed() != DVD_PLAYSPEED_NORMAL && m_av_clock->OMXPlaySpeed() != DVD_PLAYSPEED_PAUSE) {
      LOG_VERBOSE(LOG_TAG, "resume\n");
      m_playspeedCurrent = playspeed_normal;
      setSpeed(playspeeds[m_playspeedCurrent]);
      m_seekFlush = true;
   }

   setSpeed(playspeeds[m_playspeedCurrent]);
   m_av_clock->OMXResume();

#ifdef ENABLE_SUBTITLES
   if (m_has_subtitle)
      m_player_subtitles.Resume();
#endif

   LOG_INFORMATION(LOG_TAG, "Play command issued.");
   return true;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::stop
+-----------------------------------------------------------------------------*/
bool OMX_MediaProcessor::stop()
{
   log_verbose("Stop");
   QMutexLocker locker(&m_sendCmd);
   if (!checkCurrentThread())
      return false;

   switch (m_state) {
   case STATE_INACTIVE:
      return false;
   case STATE_PAUSED:
   case STATE_PLAYING:
      break;
   case STATE_STOPPED:
      return true;
   default:
      return false;
   }

   m_pendingStop = true;
   setState(STATE_STOPPED);

   // Wait for command completion.
   m_mutexPending.lock();
   if (m_pendingStop) {
      LOG_VERBOSE(LOG_TAG, "Waiting for the stop command to finish.");
      m_waitPendingCommand.wait(&m_mutexPending);
   }
   m_mutexPending.unlock();

   m_provider->cleanTextures();
   cleanup();

   LOG_INFORMATION(LOG_TAG, "Stop command issued.");
   return true;
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
      return true;
   case STATE_PAUSED:
      return true;
   case STATE_PLAYING:
      break;
   default:
      return false;
   }

#ifdef ENABLE_SUBTITLES
   if (m_has_subtitle)
      m_player_subtitles.Pause();
#endif

   setState(STATE_PAUSED);
   setSpeed(playspeeds[m_playspeedCurrent]);
   m_av_clock->OMXPause();

   // Wait for command completion.
   m_mutexPending.lock();
   if (m_pendingPause) {
      LOG_VERBOSE(LOG_TAG, "Waiting for the pause command to finish.");
      m_waitPendingCommand.wait(&m_mutexPending);
   }
   m_mutexPending.unlock();
   LOG_INFORMATION(LOG_TAG, "Pause command issued.");
   return true;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::seek
+-----------------------------------------------------------------------------*/
bool OMX_MediaProcessor::seek(qint64 position)
{
   LOG_VERBOSE(LOG_TAG, "Seek %lldms.", position);
	if (!m_av_clock)
		return false;

   // Get current position in ms.
   qint64 current  = m_av_clock->OMXMediaTime(false)*1E-3;
   qint64 currentS = qRound64(current/1000.0d);

   // position is in ms. Translate to seconds.
   int seconds = qRound64(position/1000.0d);
   int increment = seconds - currentS;
   if (!increment)
      return true;

   m_incr = increment;

   return true;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::currentPosition
+-----------------------------------------------------------------------------*/
qint64 OMX_MediaProcessor::streamPosition()
{
   if (!m_av_clock)
      return -1;
   return m_av_clock->OMXMediaTime(false)*1E-3;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::setVolume
+-----------------------------------------------------------------------------*/
void OMX_MediaProcessor::setVolume(long volume, bool linear)
{
   m_player_audio->SetCurrentVolume(volume, linear);
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::volume
+-----------------------------------------------------------------------------*/
long OMX_MediaProcessor::volume(bool linear)
{
   return m_player_audio->GetCurrentVolume(linear);
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::setMute
+-----------------------------------------------------------------------------*/
void OMX_MediaProcessor::setMute(bool muted)
{
	m_muted = muted;

	if (LIKELY(m_player_audio != NULL))
		m_player_audio->SetMuted(muted);
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::muted
+-----------------------------------------------------------------------------*/
bool OMX_MediaProcessor::muted()
{
	return m_muted;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::mediaDecoding
+-----------------------------------------------------------------------------*/
void OMX_MediaProcessor::mediaDecoding()
{
   // See description in the qmakefile.
//#define ENABLE_PROFILE_MAIN_LOOP
//#define ENABLE_PAUSE_FOR_BUFFERING

   LOG_VERBOSE(LOG_TAG, "Decoding thread started.");
   emit playbackStarted();

   // Prealloc.
#ifdef ENABLE_PAUSE_FOR_BUFFERING
   float stamp     = 0;
   float audio_pts = 0;
   float video_pts = 0;

   float audio_fifo = 0;
   float video_fifo = 0;
   float threshold = 0;
   bool audio_fifo_low = false, video_fifo_low = false, audio_fifo_high = false, video_fifo_high = false;
   float m_threshold = 1.0f; //std::min(0.1f, audio_fifo_size * 0.1f);
#endif // ENABLE_PAUSE_FOR_BUFFERING
   bool sentStarted = false;
   double last_seek_pos = 0;

   bool sendEos = false;
   double m_last_check_time = 0.0;

   m_av_clock->OMXReset(m_has_video, m_has_audio);
   m_av_clock->OMXStateExecute();
   sentStarted = true;

   while (!m_pendingStop) {
#ifdef ENABLE_PROFILE_MAIN_LOOP
      static qint64 tot    = 0;
      static qint64 totNum = 0;
      static QElapsedTimer timer;
      static int count = 0;
      if (tot == 0) {
         timer.start();
         tot++;
      }
      else
         tot += timer.restart();
      totNum++;
      if ((count++)%30 == 0) {
         //LOG_VERBOSE(LOG_TAG, "Elapsed: %lld", timer.restart());
         LOG_VERBOSE(LOG_TAG, "Average: %f.", (double)tot/totNum);
      }
#endif

      double now = m_av_clock->GetAbsoluteClock();
      bool update = false;
      if (m_last_check_time == 0.0 || m_last_check_time + DVD_MSEC_TO_TIME(20) <= now) {
         update = true;
         m_last_check_time = now;
      }

      // If a request is pending then consider done here.
      m_mutexPending.lock();
      if (m_pendingPause) {
         m_waitPendingCommand.wakeAll();
         m_pendingPause = false;
      }
      m_mutexPending.unlock();

      // TODO: Use a semaphore instead.
      if (m_state == STATE_PAUSED) {
         OMXClock::OMXSleep(2);
         continue;
      }

      if (m_seekFlush || m_incr != 0) {
         double seek_pos = 0;
         double pts      = m_av_clock->OMXMediaTime();

         //seek_pos        = (pts / DVD_TIME_BASE) + m_incr;
         seek_pos = (pts ? pts / DVD_TIME_BASE : last_seek_pos) + m_incr;
         last_seek_pos = seek_pos;

         seek_pos        *= 1000.0;

         if(m_omx_reader->SeekTime((int)seek_pos, m_incr < 0.0f, &startpts))
         {
            unsigned t = (unsigned)(startpts*1e-6);
            //auto dur = m_omx_reader->GetStreamLength() / 1000;

            log_info("Seek to: %02d:%02d:%02d\n", (t/3600), (t/60)%60, t%60);
            flushStreams(startpts);
         }

         sentStarted = false;

         if (m_omx_reader->IsEof())
            break;

         if (m_has_video && !m_player_video->Reset()) {
            m_incr = 0;
            break;
         }

         m_incr = 0;

#ifdef ENABLE_PAUSE_FOR_BUFFERING
         m_av_clock->OMXPause();
#else
         m_av_clock->OMXResume();
#endif

#ifdef ENABLE_SUBTITLES
         if (m_has_subtitle)
            m_player_subtitles.Resume();
#endif

         unsigned t = (unsigned)(startpts*1e-6);
			LOG_VERBOSE(LOG_TAG, "Seeked to: %02d:%02d:%02d", (t/3600), (t/60)%60, t%60);
         m_packetAfterSeek = false;
         m_seekFlush       = false;
      }
      else if (m_packetAfterSeek && TRICKPLAY(m_av_clock->OMXPlaySpeed())) {
         double seek_pos     = 0;
         double pts          = 0;

         pts      = m_av_clock->OMXMediaTime();
         seek_pos = (pts/DVD_TIME_BASE);

         seek_pos *= 1000.0;

#if 0
         if (m_omx_reader->SeekTime((int)seek_pos, m_av_clock->OMXPlaySpeed() < 0, &startpts))
         {
            ; //FlushStreams(DVD_NOPTS_VALUE);
         }
#endif // 0

         CLog::Log(LOGDEBUG, "Seeked %.0f %.0f %.0f\n", DVD_MSEC_TO_TIME(seek_pos), startpts, m_av_clock->OMXMediaTime());
         m_packetAfterSeek = false;
      }

      // TODO: Better error handling.
      if (m_player_audio->Error()) {
         LOG_ERROR(LOG_TAG, "Audio player error. emergency exit!");
         break;
      }

      if (update) {
#ifdef ENABLE_PAUSE_FOR_BUFFERING
         /* when the video/audio fifos are low, we pause clock, when high we resume */
         stamp     = m_av_clock->OMXMediaTime();
         audio_pts = m_player_audio->GetCurrentPTS();
         video_pts = m_player_video->GetCurrentPTS();

         if (0 && m_av_clock->OMXIsPaused()) {
            double old_stamp = stamp;
            if (audio_pts != DVD_NOPTS_VALUE && (stamp == 0 || audio_pts < stamp))
               stamp = audio_pts;
            if (video_pts != DVD_NOPTS_VALUE && (stamp == 0 || video_pts < stamp))
               stamp = video_pts;
            if (old_stamp != stamp)
            {
               m_av_clock->OMXMediaTime(stamp);
               stamp = m_av_clock->OMXMediaTime();
            }
         }

         audio_fifo = audio_pts == DVD_NOPTS_VALUE ? 0.0f : audio_pts / DVD_TIME_BASE - stamp * 1e-6;
         video_fifo = video_pts == DVD_NOPTS_VALUE ? 0.0f : video_pts / DVD_TIME_BASE - stamp * 1e-6;
         threshold  = min(0.1f, (float)m_player_audio->GetCacheTotal()*0.1f);
#endif // ENABLE_PAUSE_FOR_BUFFERING

#if 0
         static int count;
         if ((count++ & 15) == 0) {
            LOG_VERBOSE(LOG_TAG, "M: %8.02f V : %8.02f %8d %8d A : %8.02f %8.02f/%8.02f Cv : %8d Ca : %8d \r", stamp,
                        audio_fifo, m_player_video->GetDecoderBufferSize(), m_player_video->GetDecoderFreeSpace(),
                        video_fifo, m_player_audio->GetDelay(), m_player_audio->GetCacheTotal(),
                        m_player_video->GetCached(), m_player_audio->GetCached());
         }
#endif

#if 0
         if(m_tv_show_info) {
            static unsigned count;
            if ((count++ & 15) == 0) {
               char response[80];
               if (m_player_video.GetDecoderBufferSize() && m_player_audio.GetCacheTotal())
                  vc_gencmd(response, sizeof response, "render_bar 4 video_fifo %d %d %d %d",
                            (int)(100.0*m_player_video.GetDecoderBufferSize()-m_player_video.GetDecoderFreeSpace())/m_player_video.GetDecoderBufferSize(),
                            (int)(100.0*video_fifo/m_player_audio.GetCacheTotal()),
                            0, 100);
               if (m_player_audio.GetCacheTotal())
                  vc_gencmd(response, sizeof response, "render_bar 5 audio_fifo %d %d %d %d",
                            (int)(100.0*audio_fifo/m_player_audio.GetCacheTotal()),
                            (int)(100.0*m_player_audio.GetDelay()/m_player_audio.GetCacheTotal()),
                            0, 100);
               vc_gencmd(response, sizeof response, "render_bar 6 video_queue %d %d %d %d",
                         m_player_video.GetLevel(), 0, 0, 100);
               vc_gencmd(response, sizeof response, "render_bar 7 audio_queue %d %d %d %d",
                         m_player_audio.GetLevel(), 0, 0, 100);
            }
         }
#endif

#ifdef ENABLE_PAUSE_FOR_BUFFERING
         if (audio_pts != DVD_NOPTS_VALUE) {
            audio_fifo_low  = m_has_audio && audio_fifo < threshold;
            audio_fifo_high = !m_has_audio || (audio_pts != DVD_NOPTS_VALUE && audio_fifo > m_threshold);
         }

         if (video_pts != DVD_NOPTS_VALUE) {
            video_fifo_low  = m_has_video && video_fifo < threshold;
            video_fifo_high = !m_has_video || (video_pts != DVD_NOPTS_VALUE && video_fifo > m_threshold);
         }

         // Enable this to enable pause for buffering.
         if (m_state != STATE_PAUSED && (m_omx_reader->IsEof() || m_omx_pkt || TRICKPLAY(m_av_clock->OMXPlaySpeed()) || (audio_fifo_high && video_fifo_high)))
         {
            if (m_av_clock->OMXIsPaused())
            {
               CLog::Log(LOGDEBUG, "Resume %.2f,%.2f (%d,%d,%d,%d) EOF:%d PKT:%p\n", audio_fifo, video_fifo, audio_fifo_low, video_fifo_low, audio_fifo_high, video_fifo_high, m_omx_reader->IsEof(), m_omx_pkt);
               log_verbose("Pausing for buffering...");
               //m_av_clock->OMXStateExecute();
               m_av_clock->OMXResume();
            }
         }
         else if (m_state == STATE_PAUSED || audio_fifo_low || video_fifo_low)
         {
            if (!m_av_clock->OMXIsPaused())
            {
               if (m_state != STATE_PAUSED)
                 m_threshold = std::min(2.0f*m_threshold, 16.0f);
               CLog::Log(LOGDEBUG, "Pause %.2f,%.2f (%d,%d,%d,%d) %.2f\n", audio_fifo, video_fifo, audio_fifo_low, video_fifo_low, audio_fifo_high, video_fifo_high, m_threshold);
               log_verbose("Buffering completed. Resuming...");
               m_av_clock->OMXPause();
            }
         }
#endif
      }

      if (!sentStarted)
      {
         CLog::Log(LOGDEBUG, "COMXPlayer::HandleMessages - player started RESET");
         m_av_clock->OMXReset(m_has_video, m_has_audio);
         m_av_clock->OMXStateExecute();
         sentStarted = true;
      }

      if (!m_omx_pkt)
         m_omx_pkt = m_omx_reader->Read();

      if (m_omx_pkt)
         sendEos = false;

      if (m_omx_reader->IsEof() && !m_omx_pkt) {
         // demuxer EOF, but may have not played out data yet
         if ((m_has_video && m_player_video->GetCached()) ||
             (m_has_audio && m_player_audio->GetCached())) {
            OMXClock::OMXSleep(10);
            continue;
         }
         if (m_loop) {
            m_incr = m_loop_from - (m_av_clock->OMXMediaTime() ? m_av_clock->OMXMediaTime() / DVD_TIME_BASE : last_seek_pos);
            continue;
         }

         if (!sendEos && m_has_video)
            m_player_video->SubmitEOS();
         if (!sendEos && m_has_audio)
            m_player_audio->SubmitEOS();
         sendEos = true;
         if ((m_has_video && !m_player_video->IsEOS()) ||
             (m_has_audio && !m_player_audio->IsEOS()) ) {
            OMXClock::OMXSleep(10);
            continue;
         }

         break;
      }

      if(m_has_video && m_omx_pkt && m_omx_reader->IsActive(OMXSTREAM_VIDEO, m_omx_pkt->stream_index))
      {
        if (TRICKPLAY(m_av_clock->OMXPlaySpeed()))
        {
           m_packetAfterSeek = true;
        }
        if(m_player_video->AddPacket(m_omx_pkt))
          m_omx_pkt = NULL;
        else
          OMXClock::OMXSleep(10);
      }
      else if(m_has_audio && m_omx_pkt && !TRICKPLAY(m_av_clock->OMXPlaySpeed()) && m_omx_pkt->codec_type == AVMEDIA_TYPE_AUDIO)
      {
        if(m_player_audio->AddPacket(m_omx_pkt))
          m_omx_pkt = NULL;
        else
          OMXClock::OMXSleep(10);
      }
#ifdef ENABLE_SUBTITLES
      else if(m_has_subtitle && m_omx_pkt && !TRICKPLAY(m_av_clock->OMXPlaySpeed()) &&
              m_omx_pkt->codec_type == AVMEDIA_TYPE_SUBTITLE)
      {
        auto result = m_player_subtitles.AddPacket(m_omx_pkt,
                        m_omx_reader.GetRelativeIndex(m_omx_pkt->stream_index));
        if (result)
          m_omx_pkt = NULL;
        else
          OMXClock::OMXSleep(10);
      }
#endif
      else
      {
        if(m_omx_pkt)
        {
          m_omx_reader->FreePacket(m_omx_pkt);
          m_omx_pkt = NULL;
        }
        else
          OMXClock::OMXSleep(10);
      }
   }

   LOG_VERBOSE(LOG_TAG, "Stopping OMX clock...");
   m_av_clock->OMXStop();
   m_av_clock->OMXStateIdle();

   cleanup();

   setState(STATE_STOPPED);
   emit playbackCompleted();

   // Actually change the state here and reset flags.
   m_mutexPending.lock();
   if (m_pendingStop) {
      m_pendingStop = false;
      m_waitPendingCommand.wakeAll();
   }
   m_mutexPending.unlock();
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::setSpeed
+-----------------------------------------------------------------------------*/
inline
void OMX_MediaProcessor::setSpeed(int iSpeed)
{
   if (!m_av_clock)
      return;

   m_omx_reader->SetSpeed(iSpeed);

   // flush when in trickplay mode
   if (TRICKPLAY(iSpeed) || TRICKPLAY(m_av_clock->OMXPlaySpeed()))
      flushStreams(DVD_NOPTS_VALUE);

   m_av_clock->OMXSetSpeed(iSpeed);
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::flushStreams
+-----------------------------------------------------------------------------*/
void OMX_MediaProcessor::flushStreams(double pts)
{
   m_av_clock->OMXStop();
   m_av_clock->OMXPause();

   if (m_has_video)
      m_player_video->Flush();

   if (m_has_audio)
      m_player_audio->Flush();

   if (pts != DVD_NOPTS_VALUE)
       m_av_clock->OMXMediaTime(pts);

#ifdef ENABLE_SUBTITLES
   if (m_has_subtitle)
      m_player_subtitles->Flush();
#endif

   if (m_omx_pkt) {
      m_omx_reader->FreePacket(m_omx_pkt);
      m_omx_pkt = NULL;
   }

   if (pts != DVD_NOPTS_VALUE)
      m_av_clock->OMXMediaTime(pts);
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
|    OMX_VideoProcessor::convertMetaData
+-----------------------------------------------------------------------------*/
inline
void OMX_MediaProcessor::convertMetaData()
{
   // TODO: It would be good to lock this somehow.
   AVDictionary* dictionary = m_omx_reader->getMetadata();

   AVDictionaryEntry* tag = NULL;
   LOG_VERBOSE(LOG_TAG, "MetaData");
   m_metadata.clear();
   while ((tag = av_dict_get(dictionary, "", tag, AV_DICT_IGNORE_SUFFIX))) {
      m_metadata.insert(tag->key, tag->value);
      LOG_VERBOSE(LOG_TAG, "Key: %s, Value: %s.", tag->key, tag->value);
   }
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::cleanup
+-----------------------------------------------------------------------------*/
void OMX_MediaProcessor::cleanup()
{
   LOG_INFORMATION(LOG_TAG, "Cleaning up...");

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

   LOG_VERBOSE(LOG_TAG, "Closing players...");
   if (m_av_clock) {
      m_av_clock->OMXStop();
      m_av_clock->OMXStateIdle();
   }

#ifdef ENABLE_SUBTITLES
   if (m_player_subtitles)
      m_player_subtitles->Close();
#endif

   if (m_player_video)
      m_player_video->Close();
   if (m_player_audio)
      m_player_audio->Close();

   if (m_omx_pkt) {
      m_omx_reader->FreePacket(m_omx_pkt);
      m_omx_pkt = NULL;
   }

   if (m_omx_reader) {
      m_omx_reader->Close();
		//delete m_omx_reader;
		//m_omx_reader = NULL;
   }

   m_metadata.clear();
   emit metadataChanged(m_metadata);

   if (m_av_clock) {
      m_av_clock->OMXDeinitialize();
		//delete m_av_clock;
		//m_av_clock = NULL;
   }

   vc_tv_show_info(0);

   // lcarlon: free the texture. Invoke freeTexture so that it is the user
   // of the class to do it cause it is commonly required to do it in the
   // current OpenGL and EGL context. Do it here, after the stop command is
   // considered finished: this is needed to avoid hardlock in case the
   // used wants to free the texture in his own thread, which would still
   // be blocked waiting for the stop command to finish.
   LOG_VERBOSE(LOG_TAG, "Freeing texture...");
   //m_provider->free();

//#ifdef ENABLE_SUBTITLES
//   delete m_player_subtitles;
//   m_player_subtitles = NULL;
//#endif

	//delete m_player_audio;
	//delete m_player_video;
	//delete m_av_clock;

	//m_player_audio = NULL;
	//m_player_video = NULL;
	//m_av_clock = NULL;

   LOG_INFORMATION(LOG_TAG, "Cleanup done.");
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessor::streamLength
+-----------------------------------------------------------------------------*/
qint64 OMX_MediaProcessor::streamLength()
{
   if (!m_omx_reader)
      return -1;
   return m_omx_reader->GetStreamLength();
}
