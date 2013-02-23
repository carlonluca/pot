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

#ifndef OMX_TEXTURE_H
#define OMX_TEXTURE_H

/*------------------------------------------------------------------------------
|    OMX_VideoProcessorTexture class
+-----------------------------------------------------------------------------*/
#include <QSize>
#include <QSGTexture>
#include <QSGTextureProvider>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "lgl_logging.h"


/*------------------------------------------------------------------------------
|    OMX_Texture class
+-----------------------------------------------------------------------------*/
class OMX_Texture
{
public:
    OMX_Texture(EGLDisplay eglDisplay) :
        m_textureId(0), m_eglImage(0), m_eglDisplay(eglDisplay) {
        // Do nothing.
    }

    OMX_Texture(const OMX_Texture& texture) :
        m_textureId(texture.textureId()),
        m_textureSize(texture.textureSize()),
        m_eglImage(texture.eglImage()),
        m_eglDisplay(texture.eglDisplay()) {
        // Do nothing.
    }

    ~OMX_Texture() {
        resetTexture();
    }

    inline void setTexture(EGLImageKHR eglImage, uint textureId, QSize textureSize) {
        resetTexture();
        m_eglImage    = eglImage;
        m_textureId   = textureId;
        m_textureSize = textureSize;
    }

    inline void resetTexture() {
        if (m_textureId) {
            assert(m_eglImage);
            assert(m_eglDisplay);
            glDeleteTextures(1, &m_textureId);
            eglDestroyImageKHR(m_eglDisplay, m_eglImage);
        }
        m_textureId   = 0;
        m_textureSize = QSize();
    }

    inline int textureId() const {
        return m_textureId;
    }

    inline QSize textureSize() const {
        return m_textureSize;
    }

    inline EGLImageKHR eglImage() const {
        return m_eglImage;
    }

    inline EGLDisplay eglDisplay() const {
        return m_eglDisplay;
    }

    OMX_Texture& operator =(const OMX_Texture&) {
        return *this;
    }

protected:
    uint  m_textureId;
    QSize m_textureSize;
    EGLImageKHR m_eglImage;
    EGLDisplay m_eglDisplay;
};

/*------------------------------------------------------------------------------
|    OMX_SGTexture class
+-----------------------------------------------------------------------------*/
/**
 * @brief The OMX_SGTexture class Convenience class to be used in QQuickItem
 * subclass for Scene Graph.
 */
class OMX_SGTexture : public QSGTexture, public QSGTextureProvider
{
public:
    OMX_SGTexture(GLuint textureId, QSize textureSize) :
        QSGTexture()
    {
        this->m_textureId   = textureId;
        this->m_textureSize = textureSize;
    }

    inline void bind() {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
    }

    inline void setTexture(GLuint textureId, QSize textureSize) {
        LOG_DEBUG(LOG_TAG, "Setting tex: %u.", textureId);
        this->m_textureId   = textureId;
        this->m_textureSize = textureSize;
    }

    int textureId() const {
        int tex = (int)m_textureId;
        return tex;
    }

    inline QSize textureSize() const {
        return m_textureSize;
    }

    inline bool hasAlphaChannel() const {
        return false;
    }

    inline bool hasMipmaps() const {
        return false;
    }

    inline QSGTexture* texture() const {
        return (QSGTexture*)this;
    }

private:
    GLuint m_textureId;
    QSize  m_textureSize;
};

#endif // OMX_TEXTURE_H
