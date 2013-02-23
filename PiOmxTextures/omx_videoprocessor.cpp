/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.16.2012
 *
 * Copyright (c) 2012 Luca Carlon. All rights reserved.
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
#include <QFile>
#include <QGLWidget>

#include <IL/OMX_Broadcom.h>

#include "omx_videoprocessor.h"
#include "OMXComponent.h"
#include "OMX_Core.h"
#include "omxtunnel.h"
#include "lgl_logging.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
using namespace std;

#define TIMEOUT_MS 10000


/*------------------------------------------------------------------------------
|    MyEGLRenderer class
+-----------------------------------------------------------------------------*/
class MyEGLRenderer : public OMXComponent
{
protected:
   OMX_ERRORTYPE fillBufferDone(OMX_BUFFERHEADERTYPE* pBuffer) {
       assert(pBuffer);
       if (pBuffer->nFlags & OMX_BUFFERFLAG_EOS)
           return OMX_ErrorNone;
       LOG_VERBOSE(LOG_TAG, "FillBufferDoneCallback: %lu!", pBuffer->nFilledLen);
       FillThisBuffer(pBuffer);
       return OMX_ErrorNone;
   }
};
typedef std::tr1::shared_ptr<MyEGLRenderer> MyEGLRendererShared;

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::OMX_VideoProcessor
+-----------------------------------------------------------------------------*/
OMX_VideoProcessor::OMX_VideoProcessor(
        EGLDisplay eglDisplay,
        EGLContext eglContext,
        OMX_TextureProvider* provider,
        QObject* parent
        ) :
    QObject(parent),
    m_state(OMX_VideoProcessor::STATE_INACTIVE),
    m_pendingCmd(0),
    m_texture(eglDisplay),
    m_provider(provider),
    m_eglDisplay(eglDisplay),
    m_eglContext(eglContext),
    m_eglBuffer(NULL),
    m_playLock(0)
{
    // I want the object to live in a separate thread. This makes it
    // possible to decode without blocking. The other methods are supposed
    // to be called in any thread different than this (the event loop
    // will remain busy).
    moveToThread(&m_thread);
    m_thread.start();
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::~OMX_VideoProcessor
+-----------------------------------------------------------------------------*/
OMX_VideoProcessor::~OMX_VideoProcessor()
{
    m_thread.quit();
    m_thread.wait();
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::setVideoPath
+-----------------------------------------------------------------------------*/
bool OMX_VideoProcessor::setVideoPath(const QString& videoAbsolutePath)
{
    // Stop video playback and reset video about to be played. The stop
    // method is blocking, so no need to use m_pendingCmd here (also I
    // can't because that is used by stop()).
    QMutexLocker locker(&m_sendCmd);
    if (!checkCurrentThread())
        return false;

    // Validate state.
    switch (m_state) {
    case STATE_INACTIVE:
    case STATE_STOPPED:
        break;
    case STATE_PAUSED:
    case STATE_PLAYING:
        return false;
    }

    m_videoAbsolutePath = videoAbsolutePath;
    m_state = STATE_STOPPED;
    return true;
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::play
+-----------------------------------------------------------------------------*/
bool OMX_VideoProcessor::play()
{
    // I need to invoke this in another thread (this object is owned by another
    // thread).
    QMutexLocker locker(&m_sendCmd);
    if (!checkCurrentThread())
        return false;

    switch (m_state) {
    case STATE_INACTIVE:
        return false;
    case STATE_PAUSED:
        m_playLock.release();
        m_state = STATE_PLAYING;
        return true;
    case STATE_PLAYING:
        return true;
    case STATE_STOPPED:
        m_state = STATE_PLAYING;
        return QMetaObject::invokeMethod(this, "videoDecoding");
    default:
        return false;
    }
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::stop
+-----------------------------------------------------------------------------*/
bool OMX_VideoProcessor::stop()
{
    QMutexLocker locker(&m_sendCmd);
    if (!checkCurrentThread())
        return false;

    switch (m_state) {
    case STATE_INACTIVE:
        return true;
    case STATE_PAUSED:
        // TODO: What here?
        return false;
    case STATE_PLAYING:
        m_state = STATE_STOPPED;
        m_pendingCmd.acquire();
        return true;
    case STATE_STOPPED:
        return true;
    }

    return true;
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::pause
+-----------------------------------------------------------------------------*/
bool OMX_VideoProcessor::pause()
{
    QMutexLocker locker(&m_sendCmd);
    if (!checkCurrentThread())
        return false;

    switch (m_state) {
    case STATE_INACTIVE:
        return false;
    case STATE_PAUSED:
        return true;
    case STATE_PLAYING:
        m_state = STATE_PAUSED;
        m_pendingCmd.acquire();
        return true;
    case STATE_STOPPED:
        return false;
    }

    return true;
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::checkCurrentThread
+-----------------------------------------------------------------------------*/
inline
bool OMX_VideoProcessor::checkCurrentThread()
{
    if (QThread::currentThreadId() == m_thread.getThreadId()) {
        LOG_ERROR(LOG_TAG, "Do not invoke in the object's thread!");
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::checkCurrentThread
+-----------------------------------------------------------------------------*/
void OMX_VideoProcessor::videoDecoding()
{
    try {
        doVideoDecoding();
    }
    catch (const runtime_error& e) {
        LOG_ERROR(LOG_TAG, "Decoding error: %s.", e.what());
        emit playbackFailed(OMX_VideoProcessor::ERROR_FAILED_DECODING);
    }
}

/*------------------------------------------------------------------------------
|    OMX_VideoProcessor::doVideoDecoding
+-----------------------------------------------------------------------------*/
extern EGLImageKHR eglImageVideo;
void OMX_VideoProcessor::doVideoDecoding()
{
    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;

    // Try to open file. Destroying the object results in closing the file.
    QFile file(m_videoAbsolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit playbackFailed(OMX_VideoProcessor::ERROR_RESOURCE_MISSING);
        return;
    }

    // Get core.
    // TODO: Init of core must be changed.
    OMX_Core* pCore = OMX_Core::instance();
    if (!pCore) {
        emit playbackFailed(OMX_VideoProcessor::ERROR_FAILED_DECODING);
        return;
    }

    unsigned int data_len = 0;
    int packet_size       = 16 << 10;

    // Create the components.
    MyEGLRendererShared compEGLRender = OMXComponentFactory<MyEGLRenderer>::getInstance(
                "OMX.broadcom.egl_render"
                );
    OMXComponentShared compDecoder    = OMXComponentFactory<OMXComponent>::getInstance(
                "OMX.broadcom.video_decode"
                );
    OMXComponentShared compClock      = OMXComponentFactory<OMXComponent>::getInstance(
                "OMX.broadcom.clock"
                );
    OMXComponentShared compScheduler  = OMXComponentFactory<OMXComponent>::getInstance(
                "OMX.broadcom.video_scheduler"
                );

    if (!(compEGLRender && compDecoder && compClock && compScheduler)) {
        emit playbackFailed(OMX_VideoProcessor::ERROR_FAILED_DECODING);
        return;
    }

    // Disable all ports. It is necessary to do it here because otherwise
    // I'll not be able to make the components switch to IDLE state.
    LOG_VERBOSE(LOG_TAG, "Disabling all the ports...");
    compEGLRender->sendCommand(OMX_CommandPortDisable, 220, NULL);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, 220, TIMEOUT_MS);
    compEGLRender->sendCommand(OMX_CommandPortDisable, 221, NULL);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, 221, TIMEOUT_MS);
    compDecoder->sendCommand(OMX_CommandPortDisable, 130, NULL);
    compDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, 130, TIMEOUT_MS);
    compDecoder->sendCommand(OMX_CommandPortDisable, 131, NULL);
    compDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, 131, TIMEOUT_MS);
    for (int i = 80; i <= 85; i++) {
        compClock->sendCommand(OMX_CommandPortDisable, i, NULL);
        compClock->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, i, TIMEOUT_MS);
    }
    for (int i = 10; i <= 12; i++) {
        compScheduler->sendCommand(OMX_CommandPortDisable, i, NULL);
        compScheduler->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, i, TIMEOUT_MS);
    }
    LOG_VERBOSE(LOG_TAG, "All the ports were disabled.");

    // Setup clock.
    LOG_VERBOSE(LOG_TAG, "Setting params.");
    memset(&cstate, 0, sizeof(cstate));
    cstate.nSize = sizeof(cstate);
    cstate.nVersion.nVersion = OMX_VERSION;
    cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
    cstate.nWaitMask = 1;
    compClock->SetParameter(OMX_IndexConfigTimeClockState, &cstate);

    // Setup the tunnels.
    OMXTunnel tunnelDecSched(
                compDecoder.get(),
                OMXCompVideoDecoder::outputPort,
                compScheduler.get(),
                OMXCompVideoScheduler::inputVideoPort
                );
    OMXTunnel tunnelSchedRender(
                compScheduler.get(),
                OMXCompVideoScheduler::outputPort,
                compEGLRender.get(),
                220
                );
    OMXTunnel tunnelClockSched(
                compClock.get(),
                80,
                compScheduler.get(),
                OMXCompVideoScheduler::inputClockPort
                );

    LOG_VERBOSE(LOG_TAG, "Setting up tunnel clock/scheduler.");
    tunnelClockSched.setupTunnel(0, 0);
    compClock->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
    compDecoder->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);

    LOG_VERBOSE(LOG_TAG, "Setting input parameters.");
    memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 130;
    format.eCompressionFormat = OMX_VIDEO_CodingAVC;
    compDecoder->SetParameter(OMX_IndexParamVideoPortFormat, &format);

    OMX_BUFFERHEADERTYPE *buf;
    int port_settings_changed = 0;
    int first_packet          = 1;
    bool fillCalled           = false;

    // Provide buffers and enabled the port.
    compDecoder->enablePortBuffers(130);

    LOG_VERBOSE(LOG_TAG, "Changing decoder state to executing.");
    compDecoder->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);

    qint64 bytesRead = 0;
    bool releaseOnExit = false; /* Set to true in case I need to release a pending command. */
    while (compDecoder->waitForInputBufferReady(130, buf)) {
        // Check current state.
        if (m_state == OMX_VideoProcessor::STATE_INACTIVE)
            assert(false);
        else if (m_state == OMX_VideoProcessor::STATE_PAUSED) {
            // Release the pending command semaphore and wait on semaphore for
            // a play or a stop.
            m_pendingCmd.release();
            m_playLock.acquire();
        }
        else if (m_state == OMX_VideoProcessor::STATE_PLAYING) {
            // Do nothing.
        }
        else if (m_state == OMX_VideoProcessor::STATE_STOPPED) {
            releaseOnExit = true;
            break;
        }

        LOG_VERBOSE(LOG_TAG, "Filling buffer with data.");
        // feed data and wait until we get port settings changed
        unsigned char* dest = buf->pBuffer;

        data_len += file.read((char*)dest, packet_size - data_len);
        bytesRead += data_len;
        LOG_VERBOSE(LOG_TAG, "Read: %u.", data_len);

        if (port_settings_changed == 0 && compDecoder->waitForEvent(OMX_EventPortSettingsChanged, 131, 0, -1)) {
            LOG_VERBOSE(LOG_TAG, "Port settings changed event thrown!");
            port_settings_changed = 1;

            tunnelDecSched.setupTunnel(0, TIMEOUT_MS);

            LOG_VERBOSE(LOG_TAG, "Setting scheduler to EXECUTING...");
            compScheduler->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
            compScheduler->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting, TIMEOUT_MS);

            // video_render is in LOADED here.

            // now setup tunnel to video_render
            LOG_VERBOSE(LOG_TAG, "Setting up scheduler -> renderer tunnel...");
            tunnelSchedRender.setupTunnel(0, TIMEOUT_MS);

            // video_render is in IDLE here.

            // Query output buffer requirements for renderer and provide the native display
            // the renderer will use.
            OMX_PARAM_PORTDEFINITIONTYPE portdef;
            portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
            portdef.nVersion.nVersion = OMX_VERSION;
            portdef.nPortIndex = 221;
            compEGLRender->GetParameter(OMX_IndexParamPortDefinition, &portdef);
            portdef.format.video.pNativeWindow = m_eglDisplay;
            compEGLRender->SetParameter(OMX_IndexParamPortDefinition, &portdef);

            LOG_VERBOSE(LOG_TAG, "Waiting for EXECUTING state...");
            compEGLRender->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
            compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting, TIMEOUT_MS);

            // Wait for port settings changed.
            LOG_VERBOSE(LOG_TAG, "Waiting for port settings changed!");
            compEGLRender->waitForEvent(OMX_EventPortSettingsChanged, 221, 0, TIMEOUT_MS);

            // Enable output port of video_render.
            LOG_VERBOSE(LOG_TAG, "Enabling output port of EGL renderer...");
            compEGLRender->sendCommand(OMX_CommandPortEnable, 221, NULL);

            // Instead of providing a buffer I provide the EGL image to use.
            GLuint texture;
            QMetaObject::invokeMethod(
                        m_provider,
                        "instantiateTexture",
                        Qt::BlockingQueuedConnection,
                        Q_RETURN_ARG(GLuint, texture),
                        Q_ARG(QSize, QSize(1920, 1080)));
            m_texture.setTexture(eglImageVideo, texture, QSize(1920, 1080));
            emit textureReady(texture);

            LOG_VERBOSE(LOG_TAG, "Providing EGLImage: %x.", (unsigned int)eglImageVideo);
            OMX_ERRORTYPE omxErr = OMX_UseEGLImage(compEGLRender->GetHandle(), &m_eglBuffer, 221, NULL, eglImageVideo);
            if (omxErr != OMX_ErrorNone) {
                LOG_ERROR(LOG_TAG, "OpenMAXILTextureLoader::decode - OMX_UseEGLImage - failed with omxErr(0x%x)\n", omxErr);
                return;
            }

            compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, 221, TIMEOUT_MS);
            LOG_VERBOSE(LOG_TAG, "Port enabled!!!");

            // video_render is in EXECUTING here.
        }

        // File is finished.
        if (!data_len) {
            LOG_VERBOSE(LOG_TAG, "Read finished.");
            break;
        }

        buf->nFilledLen = data_len;
        data_len        = 0;
        buf->nOffset    = 0;

        if (first_packet) {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
        }
        else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

        LOG_VERBOSE(LOG_TAG, "Calling empty this buffer (%x)...", (unsigned int)buf);
        compDecoder->EmptyThisBuffer(buf);

        if (m_eglBuffer && !fillCalled) {
            m_eglBuffer->nFilledLen = 0;
            LOG_VERBOSE(LOG_TAG, "Calling FillThisBuffer for the first time.");
            compEGLRender->FillThisBuffer(m_eglBuffer);
            fillCalled = true;
        }
    }

    // Send the EOS to the decoder and wait for it to come out of the renderer.
    LOG_VERBOSE(LOG_TAG, "Rading file finished! Waiting for EOS...");
    buf->nFilledLen = 0;
    buf->nFlags     = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;
    assert(OMX_EmptyThisBuffer(compDecoder->GetHandle(), buf) == OMX_ErrorNone);
    compEGLRender->waitForEvent(OMX_EventBufferFlag, 221, OMX_BUFFERFLAG_EOS, TIMEOUT_MS);

    LOG_VERBOSE(LOG_TAG, "Flushing tunnels...");
    tunnelDecSched.flush(TIMEOUT_MS);
    tunnelSchedRender.flush(TIMEOUT_MS);
    tunnelClockSched.flush(TIMEOUT_MS);

    LOG_VERBOSE(LOG_TAG, "Disabling buffers...");
    compDecoder->disablePortBuffers(130);

    file.close();

    LOG_VERBOSE(LOG_TAG, "Disabling tunnels...");
    tunnelDecSched.disable(TIMEOUT_MS);
    tunnelSchedRender.disable(TIMEOUT_MS);
    tunnelClockSched.disable(TIMEOUT_MS);

    LOG_VERBOSE(LOG_TAG, "Go to idle...");
    compDecoder->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
    compClock->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
    compScheduler->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
    compEGLRender->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);

    compDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);
    compClock->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);
    compScheduler->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);

    // All the resources must be released.
    LOG_VERBOSE(LOG_TAG, "Go to loaded...");
    assert(OMX_FreeBuffer(compEGLRender->GetHandle(), 221, m_eglBuffer) == OMX_ErrorNone);
    m_eglBuffer = 0;
    compDecoder->sendCommand(OMX_CommandStateSet, OMX_StateLoaded, NULL);
    compClock->sendCommand(OMX_CommandStateSet, OMX_StateLoaded, NULL);
    compScheduler->sendCommand(OMX_CommandStateSet, OMX_StateLoaded, NULL);
    compEGLRender->sendCommand(OMX_CommandStateSet, OMX_StateLoaded, NULL);

    compDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateLoaded, TIMEOUT_MS);
    compClock->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateLoaded, TIMEOUT_MS);
    compScheduler->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateLoaded, TIMEOUT_MS);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateLoaded, TIMEOUT_MS);
    LOG_INFORMATION(LOG_TAG, "Decoding done! Bye bye!");

    if (releaseOnExit)
        m_pendingCmd.release();
    else
        emit playbackFinished();
}
