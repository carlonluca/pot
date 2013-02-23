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

#include <GLES2/gl2.h>


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
    virtual GLuint instantiateTexture(QSize size) = 0;
    virtual void freeTexture(GLuint textureId) = 0;
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
    GLuint instantiateTexture(QSize size);
    void freeTexture(GLuint textureId);

private:
    QQuickItem* m_item;
};

#endif // OMX_TEXTUREPROVIDERQQUICKITEM_H
