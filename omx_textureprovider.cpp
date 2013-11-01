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
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QGuiApplication>

// Private headers.
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <IL/OMX_Broadcom.h>

#include "lgl_logging.h"
#include "omx_textureprovider.h"
#include "omx_globals.h"


/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader::instantiateTexture
+-----------------------------------------------------------------------------*/
OMX_TextureData* OMX_TextureProviderQQuickItem::instantiateTexture(QSize size)
{
    EGLDisplay eglDisplay = get_egl_display();
    EGLContext eglContext = get_egl_context();

    EGLint attr[] = {EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE};

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // It seems that only 4byte pixels is supported here.
    GLubyte* pixel = new GLubyte[size.width()*size.height()*4];
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    EGLImageKHR eglImage = eglCreateImageKHR(
                eglDisplay,
                eglContext,
                EGL_GL_TEXTURE_2D_KHR,
                (EGLClientBuffer)textureId,
                attr
                );

    EGLint eglErr = eglGetError();
    if (eglErr != EGL_SUCCESS) {
        LOG_ERROR(LOG_TAG, "Failed to create KHR image: %d.", eglErr);
        return 0;
    }

    OMX_TextureData* textureData = new OMX_TextureData;
    textureData->m_textureId   = textureId;
    textureData->m_textureData = pixel;
    textureData->m_eglImage    = eglImage;
    textureData->m_textureSize = size;
    return textureData;
}

/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader::freeTexture
+-----------------------------------------------------------------------------*/
void OMX_TextureProviderQQuickItem::freeTexture(OMX_TextureData* textureData)
{
    textureData->freeData();
    delete textureData;
}
