/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.03.2012
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

/*----------------------------------------------------------------------
|    includes
+---------------------------------------------------------------------*/
#include "openmaxiltextureloader.h"

#include <QtCore/QMutexLocker>
#include <QtCore/QFile>
#include <QThread>
#include <QTime>
#include <QOpenGLContext>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <assert.h>
#include <stdio.h>

#include <IL/OMX_Broadcom.h>
extern "C" {
#include "ilclient.h"
}

#include <OMXCore.h>
#include <lgl_logging.h>
#include "omxtunnel.h"

/*----------------------------------------------------------------------
|    definitions
+---------------------------------------------------------------------*/
#define TIMEOUT_MS 10000

OpenMAXILTextureLoader* OpenMAXILTextureLoader::mInstance = 0;

/*----------------------------------------------------------------------
|    OpenMAXILTextureLoader::instance
+---------------------------------------------------------------------*/
OpenMAXILTextureLoader* OpenMAXILTextureLoader::intance() {
    return mInstance == 0 ? new OpenMAXILTextureLoader() : mInstance;
}

void OpenMAXILTextureLoader::freeInstance()
{
    delete mInstance;
    mInstance = 0;
}

/*----------------------------------------------------------------------
|    OpenMAXILTextureLoader::OpenMAXILTextureLoader
+---------------------------------------------------------------------*/
OpenMAXILTextureLoader::OpenMAXILTextureLoader() :
    eglBuffer(NULL)
{
    Q_ASSERT(mInstance == 0);
    mInstance = this;
}

/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader::loadTextureFromImage
+-----------------------------------------------------------------------------*/
/**
 * @brief OpenMAXILTextureLoader::loadTexture Loads a texture and returns its ID. The texture is loaded
 * with the data compressed in file fileAbsPath using OpenMAX.
 * @param fileAbsPath The absolute path of the file to decompress.
 * @param eglDisplay The handle to the EGL diasplay.
 * @param eglContext The handle to the EGL context.
 * @param texture The texture ID will be placed here.
 * @return True if succeds, false otherwise.
 */
bool OpenMAXILTextureLoader::loadTextureFromImage(
        QString fileAbsPath,
        EGLDisplay eglDisplay,
        EGLContext eglContext,
        GLuint& texture
        )
{
    try {
        doLoadTextureFromImage(fileAbsPath, eglDisplay, eglContext, texture);
    }
    catch (std::exception& e) {
        LOG_VERBOSE(LOG_TAG, "Error occurred: %s.", e.what());
        return false;
    }

    return true;
}

/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader::loadTexture
+-----------------------------------------------------------------------------*/
inline
void OpenMAXILTextureLoader::doLoadTextureFromImage(
        QString fileAbsPath,
        EGLDisplay eglDisplay,
        EGLContext eglContext,
        GLuint& texture
        )
{
    QFile file(fileAbsPath);
    if (!file.exists())
        throw runtime_error("File does not exist.");
    if (!file.open(QIODevice::ReadOnly))
        throw runtime_error("Failed to open file.");

    core = OMX_Core::instance();
    Q_UNUSED(core);

    // Get components.
    compImageDecoder = OMXComponentFactory<OMXComponent>::getInstance("OMX.broadcom.image_decode");
    compEGLRender    = OMXComponentFactory<OMXComponent>::getInstance("OMX.broadcom.egl_render");

    // Check the number of ports is correct.
    OMX_PORT_PARAM_TYPE port;
    port.nSize = sizeof(OMX_PORT_PARAM_TYPE);
    port.nVersion.nVersion = OMX_VERSION;
    compImageDecoder->GetParameter(OMX_IndexParamImageInit, &port);
    if (port.nPorts != 2)
        throw runtime_error("Unexpected number of ports returned from decoder.");
    m_iInPortDecode  = port.nStartPortNumber;
    m_iOutPortDecode = port.nStartPortNumber + 1;

    compEGLRender->GetParameter(OMX_IndexParamVideoInit, &port);
    if (port.nPorts != 2)
        throw runtime_error("Unexpected number of ports returned from renderer.");
    m_iInPortRender  = port.nStartPortNumber;
    m_iOutPortRender = port.nStartPortNumber + 1;

    // Disable all ports.
    LOG_VERBOSE(LOG_TAG, "Disabling all the ports...");
    compImageDecoder->sendCommand(OMX_CommandPortDisable, m_iInPortDecode, NULL);
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortDecode, TIMEOUT_MS);
    compImageDecoder->sendCommand(OMX_CommandPortDisable, m_iOutPortDecode, NULL);
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortDecode, TIMEOUT_MS);
    compEGLRender->sendCommand(OMX_CommandPortDisable, m_iInPortRender, NULL);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortRender, TIMEOUT_MS);
    compEGLRender->sendCommand(OMX_CommandPortDisable, m_iOutPortRender, NULL);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortRender, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "All the ports were disabled.");

    // move decoder component into idle state
    LOG_VERBOSE(LOG_TAG, "Changing decoder state to IDLE...");
    compImageDecoder->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "Decoder state changed to IDLE.");

    // Set input format.
    OMX_IMAGE_PARAM_PORTFORMATTYPE portFormat;
    OMX_INIT_STRUCTURE(portFormat);
    portFormat.nPortIndex = m_iInPortDecode;
    portFormat.eCompressionFormat = OMX_IMAGE_CodingJPEG;
    compImageDecoder->SetParameter(OMX_IndexParamImagePortFormat, &portFormat);

    // Query input buffer requirements.
    OMX_PARAM_PORTDEFINITIONTYPE portDefinition;
    OMX_INIT_STRUCTURE(portDefinition);
    portDefinition.nPortIndex = m_iInPortDecode;
    compImageDecoder->GetParameter(OMX_IndexParamPortDefinition, &portDefinition);

    // Enable the decoder input port.
    compImageDecoder->sendCommand(OMX_CommandPortEnable, m_iInPortDecode, NULL);

    // Provide all the buffers requested and keep a reference.
    int bufCount = portDefinition.nBufferCountActual;
    vector<OMX_BUFFERHEADERTYPE*> bufHeaders;
    for (int i = 0; i < bufCount; i++) {
        void* pBuf = 0;
        if (posix_memalign(&pBuf, portDefinition.nBufferAlignment, portDefinition.nBufferSize) != 0)
            throw runtime_error("posix_memalign failed");

        OMX_BUFFERHEADERTYPE* bufHeader = NULL;
        compImageDecoder->UseBuffer(
                    &bufHeader,
                    m_iInPortDecode,
                    (void*)i,	// the index will be our private data
                    portDefinition.nBufferSize,
                    (OMX_U8*)pBuf
                    );
        bufHeaders.push_back(bufHeader);	// add buffer header to our vector (the vector index will match 'i')
    }

#if 1
    // Providing the buffers should enabled the port. Wait now for it to be enabled.
    LOG_VERBOSE(LOG_TAG, "Waiting for input decoder port enabled.");
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iInPortDecode, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "Decoder input port enabled.");

    // Set to executing the decoder.
    LOG_VERBOSE(LOG_TAG, "Setting state of decoder to EXECUTING...");
    compImageDecoder->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "Decoder state successfully set to EXECUTING.");

    eglBuffer = NULL;
    eglImage  = NULL;

    // Start decoding.
    bool done       = false;
    bool fillCalled = false;
    int bufIndex    = 0;

    // feed source buffer into component
    while (!done) {
        // Fill this buffer.
        OMX_BUFFERHEADERTYPE* bufHeader = bufHeaders[bufIndex];
        (bufIndex + 1) >= bufCount ? bufIndex = 0 : bufIndex++;

        // Read some data to place in the buffer.
        qint64 stBytesRead = file.read((char*)bufHeader->pBuffer, bufHeader->nAllocLen);
        LOG_VERBOSE(LOG_TAG, "Read %lld bytes.", stBytesRead);
        bufHeader->nFilledLen = stBytesRead;
        bufHeader->nOffset    = 0;
        bufHeader->nFlags     = 0;

        // if we are at EOS, mark the flag.
        if (stBytesRead < bufHeader->nAllocLen) {
            LOG_VERBOSE(LOG_TAG, "Reached EOS.");
            bufHeader->nFlags = OMX_BUFFERFLAG_EOS;	// notify that the stream is done
            done = true;
        }

        compImageDecoder->EmptyThisBuffer(bufHeader);
        LOG_VERBOSE(LOG_TAG, "Called EmptyThisBuffer!");
        bool bGotEmpty = false;

        // Now it is possible to get both a part settings changed or a empty buffer done.
        while ((!bGotEmpty) || (eglBuffer == NULL)) {
            LOG_VERBOSE(LOG_TAG, "Waiting for event or empty...");
            IEventSPtr ev = compImageDecoder->WaitForEventOrEmpty(
                        OMX_EventPortSettingsChanged,
                        m_iOutPortDecode,
                        0,
                        bufHeader,
                        TIMEOUT_MS
                        );
            LOG_VERBOSE(LOG_TAG, "Got event or empty!");

            IEvent *pEv = ev.get();
            OMXEventData *pEvOMX = pEv->ToEvent();
            EmptyBufferDoneData *pEvEmpty = pEv->ToEmpty();

            if (pEvOMX != NULL) {
                // This is the case where the port settings changed event arrived.
                LOG_VERBOSE(LOG_TAG, "Got port settings changed event.");
                assert(!eglBuffer);
                onDecoderOutputChangedImage(eglDisplay, eglContext, texture);
            }
            else if (pEvEmpty != NULL) {
                // In this case this was an EmptyBufferDone event.
                LOG_VERBOSE(LOG_TAG, "got EmptyBufferDone callback.");
                bGotEmpty = true;
            }
            else
                throw runtime_error("Unexpected event.");
        }

        if (!fillCalled) {
            compEGLRender->FillThisBuffer(eglBuffer);
            fillCalled = true;
        }
    }

    // wait for fill to finish so we can grab our final buffer
    LOG_VERBOSE(LOG_TAG, "Waiting for eglBuffer to be filled...");
    compEGLRender->WaitForFill(eglBuffer, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "eglBuffer filled!");

    // wait for "end of stream" events from decoder and resizer
    LOG_VERBOSE(LOG_TAG, "Wait for EOS by output ports.");
    compImageDecoder->waitForEvent(OMX_EventBufferFlag, m_iOutPortDecode, OMX_BUFFERFLAG_EOS, TIMEOUT_MS);
    compEGLRender->waitForEvent(OMX_EventBufferFlag, m_iOutPortRender, 0x11, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "OK! Everything in EOS.");

    // at this point, we should have no events queued up at all; if we do, we have probably missed one somewhere
    assert(compImageDecoder->GetPendingEventCount() == 0);
    assert(compImageDecoder->GetPendingEmptyCount() == 0);
    assert(compImageDecoder->GetPendingFillCount() == 0);
    assert(compEGLRender->GetPendingEventCount() == 0);
    assert(compEGLRender->GetPendingEmptyCount() == 0);
    assert(compEGLRender->GetPendingFillCount() == 0);

    if (eglDestroyImageKHR(eglDisplay, eglImage) != EGL_TRUE)
        LOG_ERROR(LOG_TAG, "Failed to destroy EGL image.");

    file.close();

    // Flush tunnel.
    LOG_VERBOSE(LOG_TAG, "Flushing decoder output port.");
    compImageDecoder->sendCommand(OMX_CommandFlush, m_iOutPortDecode, NULL);
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandFlush, m_iOutPortDecode, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "Flushing render input port.");
    compEGLRender->sendCommand(OMX_CommandFlush, m_iInPortRender, NULL);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandFlush, m_iInPortRender, TIMEOUT_MS);

    // Disable input decoder port
    compImageDecoder->sendCommand(OMX_CommandPortDisable, m_iInPortDecode, NULL);
#endif

    // OMX_FreeBuffer on all input buffers
    for (vector<OMX_BUFFERHEADERTYPE *>::iterator vi = bufHeaders.begin(); vi != bufHeaders.end(); vi++) {
        void *pBuffer = (*vi)->pBuffer;
        compImageDecoder->FreeBuffer(m_iInPortDecode, *vi);
        free(pBuffer);
    }

    // Wait for disable to finish.
    LOG_VERBOSE(LOG_TAG, "Waiting for disable to finish.");
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortDecode, TIMEOUT_MS);

    // disable output resizer port
    compEGLRender->sendCommand(OMX_CommandPortDisable, m_iOutPortRender, NULL);

    // OMX_FreeBuffer on output buffer.
    compEGLRender->FreeBuffer(m_iOutPortRender, eglBuffer);

    // wait for disable to finish
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortRender, TIMEOUT_MS);

    // Disable the rest of the ports.
    LOG_VERBOSE(LOG_TAG, "Disabling ports.");
    compImageDecoder->sendCommand(OMX_CommandPortDisable, m_iOutPortDecode, NULL);
    compEGLRender->sendCommand(OMX_CommandPortDisable, m_iInPortRender, NULL);
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortDecode, TIMEOUT_MS);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortRender, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "Ports disabled.");

    // OMX_SetupTunnel with 0's to remove tunnel.
    compImageDecoder->RemoveTunnel(m_iOutPortDecode);
    compEGLRender->RemoveTunnel(m_iInPortRender);

    // Change handle states to IDLE.
    compImageDecoder->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
    compEGLRender->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);

    // Wait for state change complete.
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);

    // change handle states to LOADED
    compImageDecoder->sendCommand(OMX_CommandStateSet, OMX_StateLoaded, NULL);
    compEGLRender->sendCommand(OMX_CommandStateSet, OMX_StateLoaded, NULL);

    // wait for state change complete
    LOG_VERBOSE(LOG_TAG, "Waiting for state set to LOADED.");
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateLoaded, TIMEOUT_MS);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateLoaded, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "State LOADED.");
}

/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader::onDecoderOutputChanged
+-----------------------------------------------------------------------------*/
void OpenMAXILTextureLoader::onDecoderOutputChangedImage(
        EGLDisplay eglDisplay,
        EGLContext eglContext,
        GLuint& texture
        )
{
    // Setup the input for the render with the output of the decoder.
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    OMX_INIT_STRUCTURE(portdef);
    portdef.nPortIndex = m_iOutPortDecode;
    compImageDecoder->GetParameter(OMX_IndexParamPortDefinition, &portdef);
    portdef.nPortIndex = m_iInPortRender;
    compEGLRender->SetParameter(OMX_IndexParamPortDefinition, &portdef);

    // Tunnel output of the decoder with the input of the renderer.
    compImageDecoder->SetupTunnel(m_iOutPortDecode, compEGLRender.get(), m_iInPortRender);

    // Enable the ports involved in the tunnel.
    compImageDecoder->sendCommand(OMX_CommandPortEnable, m_iOutPortDecode, NULL);
    compEGLRender->sendCommand(OMX_CommandPortEnable, m_iInPortRender, NULL);

    // Put renderer in idle state (this allows the outport of the decoder to become enabled).
    compEGLRender->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);

    // Once the state changes, both ports should become enabled and the renderer output
    // should generate a settings changed event.
    compImageDecoder->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iOutPortDecode, TIMEOUT_MS);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iInPortRender, TIMEOUT_MS);
    compEGLRender->waitForEvent(OMX_EventPortSettingsChanged, m_iOutPortRender, 0, TIMEOUT_MS);

    // NOTE : OpenMAX official spec says that upon receving OMX_EventPortSettingsChanged event, the
    // port shall be disabled and then re-enabled (see 3.1.1.4.4 of IL v1.2.0 specification),
    // but since we have not enabled the port, I don't think we need to do anything.

    // Query output buffer requirements for renderer and provide the native display
    // the renderer will use.
    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    portdef.nVersion.nVersion = OMX_VERSION;
    portdef.nPortIndex = m_iOutPortRender;
    compEGLRender->GetParameter(OMX_IndexParamPortDefinition, &portdef);
    portdef.format.video.pNativeWindow = eglDisplay;
    compEGLRender->SetParameter(OMX_IndexParamPortDefinition, &portdef);

    // Assume this, if it's not true, more code will need to be written to handle it.
    assert(portdef.nBufferCountActual == 1);

    // Put renderer in EXECUTING and wait for port settings changed event.
    compEGLRender->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting, TIMEOUT_MS);

    // show some logging so user knows it's working
    char sLog[300];
    snprintf(sLog, sizeof(sLog), "Width: %u Height: %u Output Color Format: 0x%x Buffer Size: %u",
             (unsigned int) portdef.format.video.nFrameWidth,
             (unsigned int) portdef.format.video.nFrameHeight,
             (unsigned int) portdef.format.video.eColorFormat,
             (unsigned int) portdef.nBufferSize);
    LOG_VERBOSE(LOG_TAG, "%s\n", sLog);
    compEGLRender->waitForEvent(OMX_EventPortSettingsChanged, m_iOutPortRender, 0, TIMEOUT_MS);

    // Grab output requirements again and provide a KHR image.
    compEGLRender->GetParameter(OMX_IndexParamPortDefinition, &portdef);
    eglImage = getEGLImage(
                portdef.format.video.nFrameWidth,
                portdef.format.video.nFrameHeight,
                eglDisplay,
                eglContext,
                texture
                );

    // Enable output port of renderer.
    compEGLRender->sendCommand(OMX_CommandPortEnable, m_iOutPortRender, NULL);

    // Instead of providing a buffer I provide the EGL image to use.
    LOG_VERBOSE(LOG_TAG, "Providing EGLImage: %x.", (unsigned int)eglImage);
    OMX_ERRORTYPE omxErr = OMX_UseEGLImage(
                compEGLRender->GetHandle(),
                &eglBuffer,
                m_iOutPortRender,
                NULL,
                eglImage
                );
    if(omxErr != OMX_ErrorNone) {
        LOG_ERROR(LOG_TAG, "OpenMAXILTextureLoader::decode - OMX_UseEGLImage - failed with omxErr(0x%x)\n", omxErr);
        return;
    }
    LOG_VERBOSE(LOG_TAG, "EGLImage provided.");

    // Wait for output port enable event to be finished (it should finish once we call OMX_UseEGLImage).
    compEGLRender->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iOutPortRender, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "Renderer output port is enabled! Almost done!");
}

/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader::getEGLImage
+-----------------------------------------------------------------------------*/
inline
EGLImageKHR OpenMAXILTextureLoader::getEGLImage(
        OMX_U32 width,
        OMX_U32 height,
        EGLDisplay eglDisplay,
        EGLContext eglContext,
        GLuint& texture
        )
{
    EGLint attr[] = { EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE};

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    GLubyte* pixel = new GLubyte[width*height*4];
    memset(pixel, 0x0f, sizeof(GLubyte)*width*height*4);  // to have a grey texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    EGLImageKHR eglImage = eglCreateImageKHR(
                eglDisplay,
                eglContext,
                EGL_GL_TEXTURE_2D_KHR,
                (EGLClientBuffer)texture,
                attr
                );
    LOG_VERBOSE(LOG_TAG, "EGLImage:      %x.", (unsigned int)eglImage);
    LOG_VERBOSE(LOG_TAG, "Client buffer: %x.", texture);
    EGLint eglErr = eglGetError();
    if(eglErr != EGL_SUCCESS) {
        LOG_ERROR(LOG_TAG, "Failed to create KHR image: %d.", eglErr);
        return 0;
    }

    LOG_VERBOSE(LOG_TAG, "Successfully created KHR image: %x.", (unsigned int)eglImage);
    return eglImage;
}
