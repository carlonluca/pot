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

#include <QQuickWindow>
#include <QOpenGLContext>

// Private headers.
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <IL/OMX_Broadcom.h>

#include "lgl_logging.h"
#include "omx_textureproviderqquickitem.h"
#include "omx_globals.h"

/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader::getEGLImage
+-----------------------------------------------------------------------------*/
inline
EGLImageKHR getEGLImage(
        OMX_U32 width,
        OMX_U32 height,
        EGLDisplay eglDisplay,
        EGLContext eglContext,
        GLuint& texture
        )
{
    EGLint attr[] = {EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE};

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // TODO: Missing free of pixels.
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
    EGLint eglErr = eglGetError();
    if(eglErr != EGL_SUCCESS) {
        LOG_ERROR(LOG_TAG, "Failed to create KHR image: %d.", eglErr);
        return 0;
    }

    return eglImage;
}

// TODO: Remove this external var!
extern EGLImageKHR eglImageVideo;
GLuint OMX_TextureProviderQQuickItem::instantiateTexture(QSize size)
{
    m_item->window()->openglContext()->makeCurrent(m_item->window());
    EGLDisplay eglDisplay = get_egl_display();
    EGLContext eglContext = get_egl_context();

    // TODO: This must be passed some other way.
    GLuint texture;
    eglImageVideo = getEGLImage(size.width(), size.height(), eglDisplay, eglContext, texture);
    return texture;
}

void OMX_TextureProviderQQuickItem::freeTexture(GLuint textureId)
{
    m_item->window()->openglContext()->makeCurrent(m_item->window());
    EGLDisplay eglDisplay = get_egl_display();

    glDeleteTextures(1, &textureId);
    eglDestroyImageKHR(eglDisplay, eglImageVideo);
}
