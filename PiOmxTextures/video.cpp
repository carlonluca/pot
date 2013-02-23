/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Video deocode demo using OpenMAX IL though the ilcient helper library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QApplication>

#include "bcm_host.h"
#include "lgl_logging.h"
extern "C" {
#include "ilclient.h"
}
#include "OMXComponent.h"
#include "OMXCore.h"
#include "omxtunnel.h"

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define TIMEOUT_MS 10000

COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *video_render = NULL, *myclock = NULL;
static OMX_BUFFERHEADERTYPE* eglBuffer = NULL;

class MyEGLRenderer : public OMXComponent
{
protected:
   OMX_ERRORTYPE fillBufferDone(OMX_BUFFERHEADERTYPE* pBuffer) {
       LOG_VERBOSE(LOG_TAG, "FillBufferDoneCallback: %lu!", eglBuffer->nFilledLen);
       //OMX_FillThisBuffer(ilclient_get_handle(video_render), eglBuffer);
       assert(pBuffer == eglBuffer);
       FillThisBuffer(pBuffer);
       return OMX_ErrorNone;
   }
};
typedef shared_ptr<MyEGLRenderer> MyEGLRendererShared;

void my_fill_buffer_done(void* data, COMPONENT_T* comp)
{
    LOG_VERBOSE(LOG_TAG, "FillBufferDoneCallback: %lu!", eglBuffer->nFilledLen);
    OMX_FillThisBuffer(ilclient_get_handle(video_render), eglBuffer);
}

int video_decode_test(QString qfilename, EGLImageKHR eglImageVideo, EGLDisplay eglDisplayVideo)
{
    QString runtimeErrorDesc;
    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;

    // Try to open file.
    QFile file(qfilename);
    if (!file.open(QIODevice::ReadOnly)) {
        runtimeErrorDesc = "Failed to open file.";
        // TODO: Cleanup.
        //goto omx_init_failed;
    }

    // Get core.
    OMX_Core* pCore    = OMX_Core::instance();
    if (!pCore) {
        runtimeErrorDesc = "Failed to get OMX core.";
        // TODO: Cleanup.
        //goto omx_init_failed;
    }

    unsigned char* data   = NULL;
    unsigned int data_len = 0;
    int find_start_codes  = 0;
    int packet_size       = 16 << 10;

    // TODO: Is this even used?!
    if (find_start_codes && (data = (unsigned char*)malloc(packet_size + 4)) == NULL) {
        assert(false);
#if 0
        status = -16;
        if(OMX_Deinit() != OMX_ErrorNone)
            status = -17;
        ilclient_destroy(client);
        fclose(in);
        return status;
#endif
    }

    // Create the components.
    LOG_DEBUG(LOG_TAG, "Getting components.");
    MyEGLRendererShared compEGLRender = OMXComponentFactory<MyEGLRenderer>::getInstance("OMX.broadcom.egl_render");
    OMXComponentShared compDecoder    = OMXComponentFactory<OMXComponent>::getInstance("OMX.broadcom.video_decode");
    OMXComponentShared compClock      = OMXComponentFactory<OMXComponent>::getInstance("OMX.broadcom.clock");
    OMXComponentShared compScheduler  = OMXComponentFactory<OMXComponent>::getInstance("OMX.broadcom.video_scheduler");

    LOG_DEBUG(LOG_TAG, "Got components.");

    if (!(compEGLRender && compDecoder && compClock && compScheduler)) {
        runtimeErrorDesc = "Failed to get components.";
        // TODO: Cleanup.
        //goto omx_init_failed;
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
    compClock->SetParameter(OMX_IndexConfigTimeClockState, &cstate); // throws runtime_exception.

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
#if 0
    LOG_VERBOSE(LOG_TAG, "Setting up tunnels...");
    compVideoDecoder->SetupTunnel(
                OMXCompVideoDecoder::outputPort,
                compScheduler,
                OMXCompVideoScheduler::inputVideoPort
                );
    compScheduler->SetupTunnel(
                OMXCompVideoScheduler::outputPort,
                compEGLRender,
                220
                ); // TODO: Create EGL render component.
    compClock->SetupTunnel(20, compScheduler, 12);
#endif

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
    while (compDecoder->waitForInputBufferReady(130, buf)) {
        LOG_VERBOSE(LOG_TAG, "Filling buffer with data.");
        // feed data and wait until we get port settings changed
        unsigned char* dest = find_start_codes ? data + data_len : buf->pBuffer;

        //data_len += fread(dest, 1, packet_size+(find_start_codes*4)-data_len, in);
        data_len += file.read((char*)dest, packet_size+(find_start_codes*4)-data_len);
        bytesRead += data_len;

        LOG_DEBUG(LOG_TAG, "Read bytes: %u.", data_len);

        try {
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

#if 0
                {
                    assert(OMX_SendCommand(ILC_GET_HANDLE(video_render), OMX_CommandPortDisable, 221, NULL) == OMX_ErrorNone);
                    // Setup the input for the renderer with the output of the scheduler.
                    OMX_PARAM_PORTDEFINITIONTYPE portdef;
                    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
                    portdef.nVersion.nVersion = OMX_VERSION;
                    portdef.nPortIndex = 11;
                    assert(OMX_GetParameter(ILC_GET_HANDLE(video_scheduler), OMX_IndexParamPortDefinition, &portdef) == OMX_ErrorNone);
                    //m_pCompDecode->GetParameter(OMX_IndexParamPortDefinition, &portdef);
                    portdef.nPortIndex = 220;
                    LOG_DEBUG(LOG_TAG, "Error: %x.", OMX_SetParameter(ILC_GET_HANDLE(video_render), OMX_IndexParamPortDefinition, &portdef));
                    //m_pCompRender->SetParameter(OMX_IndexParamPortDefinition, &portdef);
                    assert(OMX_SendCommand(ILC_GET_HANDLE(video_render), OMX_CommandPortEnable, 221, NULL) == OMX_ErrorNone);
                }
#endif

                // video_render is in IDLE here.

                // Query output buffer requirements for renderer and provide the native display
                // the renderer will use.
                OMX_PARAM_PORTDEFINITIONTYPE portdef;
                portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
                portdef.nVersion.nVersion = OMX_VERSION;
                portdef.nPortIndex = 221;
                compEGLRender->GetParameter(OMX_IndexParamPortDefinition, &portdef);
                portdef.format.video.pNativeWindow = eglDisplayVideo;
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
                LOG_VERBOSE(LOG_TAG, "Providing EGLImage: %x.", (unsigned int)eglImageVideo);
                OMX_ERRORTYPE omxErr = OMX_UseEGLImage(compEGLRender->GetHandle(), &eglBuffer, 221, NULL, eglImageVideo);
                if(omxErr != OMX_ErrorNone) {
                    LOG_ERROR(LOG_TAG, "OpenMAXILTextureLoader::decode - OMX_UseEGLImage - failed with omxErr(0x%x)\n", omxErr);
                    return -1;
                }

                compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, 221, TIMEOUT_MS);
                LOG_VERBOSE(LOG_TAG, "Port enabled!!!");

                // video_render is in EXECUTING here.
            }
        }
        catch (const runtime_error& err) {
            Q_UNUSED(err);
            LOG_ERROR(LOG_TAG, "Exception: %s.", err.what());
            // Do nothing and go on.
        }

        // File is finished.
        if (!data_len)
            break;

        // ???
        if (find_start_codes) {
            int i, start = -1, len = 0;
            int max_len = data_len > packet_size ? packet_size : data_len;
            for(i = 2; i < max_len; i++) {
                if (data[i-2] == 0 && data[i-1] == 0 && data[i] == 1) {
                    len = 3;
                    start = i-2;

                    // check for 4 byte start code
                    if(i > 2 && data[i-3] == 0) {
                        len++;
                        start--;
                    }

                    break;
                }
            }

            if (start == 0) {
                // start code is next, so just send that
                buf->nFilledLen = len;
            }
            else if(start == -1) {
                // no start codes seen, send the first block
                buf->nFilledLen = max_len;
            }
            else {
                // start code in the middle of the buffer, send up to the code
                buf->nFilledLen = start;
            }

            memcpy(buf->pBuffer, data, buf->nFilledLen);
            memmove(data, data + buf->nFilledLen, data_len - buf->nFilledLen);
            data_len -= buf->nFilledLen;
        }
        else {
            buf->nFilledLen = data_len;
            data_len = 0;
        }

#if 1
        buf->nOffset = 0;
        if (first_packet) {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
        }
        else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
#endif

        LOG_VERBOSE(LOG_TAG, "Calling empty this buffer (%x)...", buf);
        compDecoder->EmptyThisBuffer(buf);

#if 1
        if (eglBuffer && !fillCalled) {
            eglBuffer->nFilledLen = 0;
            LOG_VERBOSE(LOG_TAG, "Calling FillThisBuffer for the first time.");
            //assert(OMX_FillThisBuffer(ILC_GET_HANDLE(video_render), eglBuffer) == OMX_ErrorNone);
            compEGLRender->FillThisBuffer(eglBuffer);
            fillCalled = true;
        }
#endif
    }

    qApp->exit();
    return 0;
#if 0
    QByteArray baFilename = qfilename.toLocal8Bit();
    char* filename = baFilename.data();
    bcm_host_init();

    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
    COMPONENT_T *list[5];
    TUNNEL_T tunnel[4];
    ILCLIENT_T *client;
    FILE *in;
    int status = 0;
    unsigned char *data = NULL;
    unsigned int data_len = 0;
    int find_start_codes = 0;
    int packet_size = 16<<10;

    memset(list, 0, sizeof(list));
    memset(tunnel, 0, sizeof(tunnel));

    if((in = fopen(filename, "rb")) == NULL)
        return -2;

    if((client = ilclient_init()) == NULL)
    {
        fclose(in);
        return -3;
    }
    ilclient_set_fill_buffer_done_callback(client, my_fill_buffer_done, 0);

    if(OMX_Init() != OMX_ErrorNone)
    {
        ilclient_destroy(client);
        fclose(in);
        return -4;
    }

    if(find_start_codes && (data = (unsigned char*)malloc(packet_size+4)) == NULL)
    {
        status = -16;
        if(OMX_Deinit() != OMX_ErrorNone)
            status = -17;
        ilclient_destroy(client);
        fclose(in);
        return status;
    }
    // create video_decode
    if(ilclient_create_component(client, &video_decode, "video_decode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
        status = -14;
    list[0] = video_decode;

    // create video_render
    if(status == 0 && ilclient_create_component(client, &video_render, "egl_render", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_ENABLE_OUTPUT_BUFFERS | ILCLIENT_DISABLE_ALL_PORTS)) != 0)
        status = -14;
    list[1] = video_render;

    // create clock
    if(status == 0 && ilclient_create_component(client, &myclock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
        status = -14;
    list[2] = myclock;

    memset(&cstate, 0, sizeof(cstate));
    cstate.nSize = sizeof(cstate);
    cstate.nVersion.nVersion = OMX_VERSION;
    cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
    cstate.nWaitMask = 1;
    if(myclock != NULL && OMX_SetParameter(ILC_GET_HANDLE(myclock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
        status = -13;

    // create video_scheduler
    if(status == 0 && ilclient_create_component(client, &video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
        status = -14;
    list[3] = video_scheduler;

    qDebug("Setting tunnels...");
    set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
    set_tunnel(tunnel+1, video_scheduler, 11, video_render, 220);
    set_tunnel(tunnel+2, myclock, 80, video_scheduler, 12);

    // setup clock tunnel first
    if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
        status = -15;
    else
        ilclient_change_component_state(myclock, OMX_StateExecuting);
    qDebug("Clock now in EXECUTING.");

    if(status == 0)
        ilclient_change_component_state(video_decode, OMX_StateIdle);
    qDebug("Decoder now in IDLE.");

    memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 130;
    format.eCompressionFormat = OMX_VIDEO_CodingAVC;

    bool fillCalled = false;
    if(status == 0 &&
            OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
            ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
    {
        OMX_BUFFERHEADERTYPE *buf;
        int port_settings_changed = 0;
        int first_packet = 1;

        ilclient_change_component_state(video_decode, OMX_StateExecuting);
        qDebug("Decoder in EXECUTING.");

        long bytesRead = 0;
        while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL)
        {
            // feed data and wait until we get port settings changed
            unsigned char *dest = find_start_codes ? data + data_len : buf->pBuffer;

            data_len += fread(dest, 1, packet_size+(find_start_codes*4)-data_len, in);
            bytesRead += data_len;

            LOG_DEBUG(LOG_TAG, "Read bytes: %lu.", bytesRead);

            if(port_settings_changed == 0 &&
                    ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
                     (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                               ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
            {
                qDebug("Port settings changed for decoder.");
                port_settings_changed = 1;

                if(ilclient_setup_tunnel(tunnel, 0, 0) != 0)
                {
                    status = -7;
                    break;
                }

                ilclient_change_component_state(video_scheduler, OMX_StateExecuting);


                // video_render is in LOADED here.

                // now setup tunnel to video_render
                if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
                {
                    status = -12;
                    break;
                }

#if 0
                {
                    assert(OMX_SendCommand(ILC_GET_HANDLE(video_render), OMX_CommandPortDisable, 221, NULL) == OMX_ErrorNone);
                    // Setup the input for the renderer with the output of the scheduler.
                    OMX_PARAM_PORTDEFINITIONTYPE portdef;
                    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
                    portdef.nVersion.nVersion = OMX_VERSION;
                    portdef.nPortIndex = 11;
                    assert(OMX_GetParameter(ILC_GET_HANDLE(video_scheduler), OMX_IndexParamPortDefinition, &portdef) == OMX_ErrorNone);
                    //m_pCompDecode->GetParameter(OMX_IndexParamPortDefinition, &portdef);
                    portdef.nPortIndex = 220;
                    LOG_DEBUG(LOG_TAG, "Error: %x.", OMX_SetParameter(ILC_GET_HANDLE(video_render), OMX_IndexParamPortDefinition, &portdef));
                    //m_pCompRender->SetParameter(OMX_IndexParamPortDefinition, &portdef);
                    assert(OMX_SendCommand(ILC_GET_HANDLE(video_render), OMX_CommandPortEnable, 221, NULL) == OMX_ErrorNone);
                }
#endif

                // video_render is in IDLE here.

                // Query output buffer requirements for renderer and provide the native display
                // the renderer will use.
                OMX_PARAM_PORTDEFINITIONTYPE portdef;
                portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
                portdef.nVersion.nVersion = OMX_VERSION;
                portdef.nPortIndex = 221;
                OMX_GetParameter(ILC_GET_HANDLE(video_render), OMX_IndexParamPortDefinition, &portdef);
                //m_pCompRender->GetParameter(OMX_IndexParamPortDefinition, &portdef);
                portdef.format.video.pNativeWindow = eglDisplayVideo;
                assert(OMX_SetParameter(ILC_GET_HANDLE(video_render), OMX_IndexParamPortDefinition, &portdef) == OMX_ErrorNone);
                //m_pCompRender->SetParameter(OMX_IndexParamPortDefinition, &portdef);

                printf("Waiting for EXECUTING state...\n");
                //ilclient_change_component_state(video_render, OMX_StateExecuting);
                assert(OMX_SendCommand(ILC_GET_HANDLE(video_render), OMX_CommandStateSet, OMX_StateExecuting, NULL) == OMX_ErrorNone);
                ilclient_wait_for_command_complete(video_render, OMX_CommandStateSet, OMX_StateExecuting);
                printf("Ok with executing...\n");

                // Wait for port settings changed.
                ilclient_wait_for_event(video_render, OMX_EventPortSettingsChanged, 221, 0, 0, 1, ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000);
                LOG_VERBOSE(LOG_TAG, "Yuhuuuu! Port settings changed!");

                // Enable output port of video_render.
                LOG_VERBOSE(LOG_TAG, "Enabling port...");
                assert(OMX_SendCommand(ILC_GET_HANDLE(video_render), OMX_CommandPortEnable, 221, NULL) == OMX_ErrorNone);

                // Instead of providing a buffer I provide the EGL image to use.
                LOG_VERBOSE(LOG_TAG, "Providing EGLImage: %x.", (unsigned int)eglImageVideo);
                //OMX_ERRORTYPE omxErr = OMX_UseEGLImage(m_pCompRender->GetHandle(), &eglBuffer, m_iOutPortRender, NULL, eglImage);
                OMX_ERRORTYPE omxErr = OMX_UseEGLImage(ILC_GET_HANDLE(video_render), &eglBuffer, 221, NULL, eglImageVideo);
                if(omxErr != OMX_ErrorNone) {
                    LOG_ERROR(LOG_TAG, "OpenMAXILTextureLoader::decode - OMX_UseEGLImage - failed with omxErr(0x%x)\n", omxErr);
                    return -1;
                }
                LOG_VERBOSE(LOG_TAG, "EGLImage provided.");

                ilclient_wait_for_command_complete(video_render, OMX_CommandPortEnable, 221);
                LOG_VERBOSE(LOG_TAG, "Port enabled!!!");

                // video_render is in EXECUTING here.
            }

            if(!data_len)
                break;

            if(find_start_codes)
            {
                int i, start = -1, len = 0;
                int max_len = data_len > packet_size ? packet_size : data_len;
                for(i=2; i<max_len; i++)
                {
                    if(data[i-2] == 0 && data[i-1] == 0 && data[i] == 1)
                    {
                        len = 3;
                        start = i-2;

                        // check for 4 byte start code
                        if(i > 2 && data[i-3] == 0)
                        {
                            len++;
                            start--;
                        }

                        break;
                    }
                }

                if(start == 0)
                {
                    // start code is next, so just send that
                    buf->nFilledLen = len;
                }
                else if(start == -1)
                {
                    // no start codes seen, send the first block
                    buf->nFilledLen = max_len;
                }
                else
                {
                    // start code in the middle of the buffer, send up to the code
                    buf->nFilledLen = start;
                }

                memcpy(buf->pBuffer, data, buf->nFilledLen);
                memmove(data, data + buf->nFilledLen, data_len - buf->nFilledLen);
                data_len -= buf->nFilledLen;
            }
            else
            {
                buf->nFilledLen = data_len;
                data_len = 0;
            }

            buf->nOffset = 0;
            if(first_packet)
            {
                buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
                first_packet = 0;
            }
            else
                buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

            LOG_VERBOSE(LOG_TAG, "Calling empty this buffer...");
            if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
            {
                status = -6;
                break;
            }

            if (eglBuffer && !fillCalled) {
                eglBuffer->nFilledLen = 0;
                LOG_VERBOSE(LOG_TAG, "Calling FillThisBuffer for the first time.");
                assert(OMX_FillThisBuffer(ILC_GET_HANDLE(video_render), eglBuffer) == OMX_ErrorNone);
                fillCalled = true;
            }
        }

        LOG_INFORMATION(LOG_TAG, "Read finished!");
        buf->nFilledLen = 0;
        buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

        if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
            status = -20;

        // wait for EOS from render
        LOG_VERBOSE(LOG_TAG, "Waiting for EOS from renderer...");
        ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
                                ILCLIENT_BUFFER_FLAG_EOS, 10000);
        //sleep(50);
        LOG_VERBOSE(LOG_TAG, "EOS arrived!");

        // need to flush the renderer to allow video_decode to disable its input port
        ilclient_flush_tunnels(tunnel, 0);

        ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
    }

    fclose(in);

    ilclient_disable_tunnel(tunnel);
    ilclient_disable_tunnel(tunnel+1);
    ilclient_disable_tunnel(tunnel+2);
    ilclient_teardown_tunnels(tunnel);

    ilclient_state_transition(list, OMX_StateIdle);
    ilclient_state_transition(list, OMX_StateLoaded);

    ilclient_cleanup_components(list);

    OMX_Deinit();

    ilclient_destroy(client);

    LOG_INFORMATION(LOG_TAG, "All cleaned up!");
    return status;
#endif
}

#if 0
int main (int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    return video_decode_test(argv[1]);
}
#endif
