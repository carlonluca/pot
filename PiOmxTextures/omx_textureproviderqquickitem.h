/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.19.2012
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

#ifndef OMX_TEXTUREPROVIDERQQUICKITEM_H
#define OMX_TEXTUREPROVIDERQQUICKITEM_H

/*------------------------------------------------------------------------------
|    OMX_VideoProcessorThread class
+-----------------------------------------------------------------------------*/
#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>

#include <GLES2/gl2.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <assert.h>

#include "lgl_logging.h"
#include "omx_globals.h"


/*------------------------------------------------------------------------------
|    OMX_TextureData class
+-----------------------------------------------------------------------------*/
class OMX_TextureData
{
public:
    OMX_TextureData() :
        m_textureId(0),
        m_textureData(NULL),
        m_eglImage(NULL),
        m_textureSize(QSize(0, 0)) {
        // Do nothing.
    }

    OMX_TextureData(const OMX_TextureData& textureData) :
        m_textureId(textureData.m_textureId),
        m_textureData(textureData.m_textureData),
        m_eglImage(textureData.m_eglImage),
        m_textureSize(textureData.m_textureSize) {
        // Do nothing.
    }

    ~OMX_TextureData() {
        // Do not free.
    }

    void freeData() {
        EGLDisplay eglDisplay = get_egl_display();

        // Destroy texture, EGL image and free the buffer.
        if (eglDestroyImageKHR(eglDisplay, m_eglImage) == EGL_SUCCESS) {
            EGLint err = eglGetError();
            LOG_ERROR(LOG_TAG, "Failed to destroy EGLImageKHR: %d.", err);
        }
        glDeleteTextures(1, &m_textureId);
        delete m_textureData;
    }

    GLuint      m_textureId;
    GLubyte*    m_textureData;
    EGLImageKHR m_eglImage;
    QSize       m_textureSize;
};

/*------------------------------------------------------------------------------
|    OMX_VideoProcessorThread class
+-----------------------------------------------------------------------------*/
/**
 * @brief The OMX_TextureProviderQQuickItem class can be instantiate by
 * a QQuickItem and passed to a OMX element. It MUST be instantiated in
 * the renderer thread.
 */
class OMX_TextureProvider : public QObject
{
    Q_OBJECT
public slots:
    virtual OMX_TextureData* instantiateTexture(QSize size) = 0;
    virtual void freeTexture(OMX_TextureData* textureData) = 0;
};

/*------------------------------------------------------------------------------
|    OMX_TextureProviderQQuickItem class
+-----------------------------------------------------------------------------*/
class OMX_TextureProviderQQuickItem : public OMX_TextureProvider
{
    Q_OBJECT
public:
    OMX_TextureProviderQQuickItem(QQuickItem* item) :
        OMX_TextureProvider(),
        m_item(item) {
        // Do nothing.
    }

public slots:
    OMX_TextureData* instantiateTexture(QSize size);
    void freeTexture(OMX_TextureData* textureData);

private:
    QQuickItem* m_item;
};

#endif // OMX_TEXTUREPROVIDERQQUICKITEM_H
