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

#ifndef OPENMAXILTEXTURELOADER_H
#define OPENMAXILTEXTURELOADER_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QtCore/QObject>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Image.h>
#include <IL/OMX_Component.h>

#include "OMXComponent.h"
#include "OMX_Core.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#define OMX_INIT_STRUCTURE(a) \
    memset(&(a), 0, sizeof(a)); \
    (a).nSize = sizeof(a); \
    (a).nVersion.nVersion = OMX_VERSION; \
    (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
    (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
    (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
    (a).nVersion.s.nStep = OMX_VERSION_STEP


/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader class
+-----------------------------------------------------------------------------*/
class OpenMAXILTextureLoader
{
public:
    static OpenMAXILTextureLoader* intance();
    static void freeInstance();
    bool loadTextureFromImage(
            QString fileAbsPath,
            EGLDisplay eglDisplay,
            EGLContext eglContext,
            GLuint& texture
            );
    bool loadTextureFromVideo(
            QString fileAbsPath,
            EGLDisplay eglDisplay,
            EGLContext eglContext,
            GLuint& texture
            );

private:
    explicit OpenMAXILTextureLoader();
    static OpenMAXILTextureLoader* mInstance;

    void doLoadTextureFromVideo(
            QString fileAbsPath,
            EGLDisplay eglDisplay,
            EGLContext eglContext,
            GLuint& texture
            );
    void onDecoderOutputChangedVideo(EGLDisplay eglDisplay, EGLContext eglContext, GLuint &texture);

    void doLoadTextureFromImage(
            QString fileAbsPath,
            EGLDisplay eglDisplay,
            EGLContext eglContext,
            GLuint& texture
            );
    void onDecoderOutputChangedImage(EGLDisplay eglDisplay, EGLContext eglContext, GLuint& texture);

    EGLImageKHR getEGLImage(
            OMX_U32 width,
            OMX_U32 height,
            EGLDisplay eglDisplay,
            EGLContext eglContext,
            GLuint& texture
            );

    OMX_Core* core;

    // Image decoder.
    OMXComponentShared compImageDecoder;
    int m_iInPortDecode, m_iOutPortDecode;

    // EGL renderer.
    OMXComponentShared compEGLRender;
    int m_iInPortRender, m_iOutPortRender;
    OMX_BUFFERHEADERTYPE* eglBuffer;
    EGLImageKHR eglImage;
};

#endif // OPENMAXILTEXTURELOADER_H
