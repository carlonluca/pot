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

#include <IL/OMX_Broadcom.h>
#include <EGL/egl.h>

#include "omx_videograph.h"
#include "omx_globals.h"


#define TIMEOUT_MS 10000


OMX_VideoGraph::OMX_VideoGraph(
        OMX_TextureProvider* provider,
        QObject* parent
        ) :
    m_eglDisplay(get_egl_display()),
    m_eglContext(get_egl_context()),
    m_provider(provider),
    m_eglBuffer(NULL),
    m_texture(m_eglDisplay),
    port_settings_changed(0),
    first_packet(1),
    fillCalled(false)
{
    // Get core.
    // TODO: Init of core must be changed.
    m_core = OMX_Core::instance();
    if (!m_core)
        throw OMX_Exception("Failed to init core.");

    // Create the components.
    LOG_VERBOSE(LOG_TAG, "Instantiating components...");
    compEGLRender = OMXComponentFactory<MyEGLRenderer>::getInstance(
                "OMX.broadcom.egl_render"
                );
    compDecoder = OMXComponentFactory<OMXComponent>::getInstance(
                "OMX.broadcom.video_decode"
                );
    compClock = OMXComponentFactory<OMXComponent>::getInstance(
                "OMX.broadcom.clock"
                );
    compScheduler = OMXComponentFactory<OMXComponent>::getInstance(
                "OMX.broadcom.video_scheduler"
                );

    if (!(compEGLRender && compDecoder && compClock && compScheduler))
        throw OMX_Exception("Failed to create component.");

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
    OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
    memset(&cstate, 0, sizeof(cstate));
    cstate.nSize = sizeof(cstate);
    cstate.nVersion.nVersion = OMX_VERSION;
    cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
    cstate.nWaitMask = 1;
    compClock->SetParameter(OMX_IndexConfigTimeClockState, &cstate);

    // Setup the tunnels.
    tunnelDecSched = new OMXTunnel(
                compDecoder.get(),
                OMXCompVideoDecoder::outputPort,
                compScheduler.get(),
                OMXCompVideoScheduler::inputVideoPort
                );
    tunnelSchedRender = new OMXTunnel(
                compScheduler.get(),
                OMXCompVideoScheduler::outputPort,
                compEGLRender.get(),
                220
                );
    tunnelClockSched = new OMXTunnel(
                compClock.get(),
                80,
                compScheduler.get(),
                OMXCompVideoScheduler::inputClockPort
                );

    LOG_VERBOSE(LOG_TAG, "Setting up tunnel clock/scheduler.");
    tunnelClockSched->setupTunnel(0, 0);
    compClock->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
    compDecoder->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);

    LOG_VERBOSE(LOG_TAG, "Setting input parameters.");
    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 130;
    format.eCompressionFormat = OMX_VIDEO_CodingAVC;
    compDecoder->SetParameter(OMX_IndexParamVideoPortFormat, &format);

    OMX_NALSTREAMFORMATTYPE nalFormat;
    OMX_INIT_STRUCTURE(nalFormat);
    nalFormat.nPortIndex = 130;
    //assert(OMX_GetParameter(compDecoder->GetHandle(), , &nalFormat) == OMX_ErrorNone);
    //compDecoder->GetParameter(OMX_IndexParamNalStreamFormatSupported, &nalFormat);
    nalFormat.eNaluFormat = OMX_NaluFormatOneNaluPerBuffer;
    compDecoder->SetParameter((OMX_INDEXTYPE)OMX_IndexParamNalStreamFormatSelect, &nalFormat);

    //assert(false);

    // Provide buffers and enabled the port.
    compDecoder->enablePortBuffers(130);

    LOG_VERBOSE(LOG_TAG, "Changing decoder state to executing.");
    compDecoder->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
}

OMX_VideoGraph::~OMX_VideoGraph()
{
#if 0
    delete tunnelDecSched;
    delete tunnelSchedRender;
    delete tunnelClockSched;

    OMX_Core::destroyInstance();
    m_core = NULL;
#endif
}

bool OMX_VideoGraph::getAvailableBuffer(OMX_BUFFERHEADERTYPE** buffer)
{
    // Failure here means something bad happened.
    return compDecoder->waitForInputBufferReady(130, *buffer);
}

extern EGLImageKHR eglImageVideo;
bool OMX_VideoGraph::playData(OMX_BUFFERHEADERTYPE*& buf)
{
    if (port_settings_changed == 0 && compDecoder->waitForEvent(OMX_EventPortSettingsChanged, 131, 0, -1)) {
        LOG_VERBOSE(LOG_TAG, "Port settings changed event thrown!");
        port_settings_changed = 1;

        tunnelDecSched->setupTunnel(0, TIMEOUT_MS);

        LOG_VERBOSE(LOG_TAG, "Setting scheduler to EXECUTING...");
        compScheduler->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
        compScheduler->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting, TIMEOUT_MS);

        // video_render is in LOADED here.

        // now setup tunnel to video_render
        LOG_VERBOSE(LOG_TAG, "Setting up scheduler -> renderer tunnel...");
        tunnelSchedRender->setupTunnel(0, TIMEOUT_MS);

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
            LOG_ERROR(LOG_TAG, "OpenMAXILTextureLoader::decode - OMX_UseEGLImage - failed with omxErr(0x%x)\n",
                      omxErr);
            return false;
        }

        compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, 221, TIMEOUT_MS);
        LOG_VERBOSE(LOG_TAG, "Port enabled!!!");

        // video_render is in EXECUTING here.
    }

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

    return true;
}
