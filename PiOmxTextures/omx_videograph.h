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

#ifndef OMX_VIDEOGRAPH_H
#define OMX_VIDEOGRAPH_H

#include <string>
#include <assert.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "OMX_Core.h"
#include "omxtunnel.h"
#include "lgl_logging.h"
#include "omx_textureproviderqquickitem.h"
#include "omx_texture.h"

using namespace std;


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
|    OMX_Exception class
+-----------------------------------------------------------------------------*/
class OMX_Exception : public runtime_error
{
public:
    OMX_Exception(const string& message) : runtime_error(message) {
        // Do nothing.
    }
};

/*------------------------------------------------------------------------------
|    OMX_VideoGraph class
+-----------------------------------------------------------------------------*/
class OMX_VideoGraph : public QObject
{
    Q_OBJECT
public:
    OMX_VideoGraph(
            OMX_TextureProvider* provider,
            QObject* parent = 0
            );
    ~OMX_VideoGraph();

    bool getAvailableBuffer(OMX_BUFFERHEADERTYPE** buffer);
    bool playData(OMX_BUFFERHEADERTYPE*& buf);

signals:
    void textureReady(const GLuint& textureId);

private:
    // EGL/OpenGL data.
    EGLDisplay m_eglDisplay;
    EGLContext m_eglContext;
    OMX_TextureProvider* m_provider;
    OMX_Texture m_texture;
    OMX_BUFFERHEADERTYPE* m_eglBuffer;

    OMX_Core* m_core;

    // Components.
    MyEGLRendererShared compEGLRender;
    OMXComponentShared compDecoder;
    OMXComponentShared compClock;
    OMXComponentShared compScheduler;

    // Tunnels.
    OMXTunnel* tunnelDecSched;
    OMXTunnel* tunnelSchedRender;
    OMXTunnel* tunnelClockSched;

    int port_settings_changed;
    int first_packet;
    bool fillCalled;
};

#endif // OMX_VIDEOGRAPH_H
