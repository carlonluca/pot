/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.18.2012
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

#ifndef OMX_TEXTUREDELEMENT_H
#define OMX_TEXTUREDELEMENT_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QtQuick/QQuickItem>
#include <QOpenGLContext>
#include <QSGGeometryNode>


/*------------------------------------------------------------------------------
|    OMX_ImageElement class
+-----------------------------------------------------------------------------*/
class OMX_TexturedElement : public QQuickItem
{
    Q_OBJECT
public:
    OMX_TexturedElement(QQuickItem* parent = 0);
    ~OMX_TexturedElement();

    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

    uint textureId() {
        return m_texture;
    }

    void setTextureId(const uint& textureId) {
        // Set the new texture ID and update.
        // TODO: Who is freeing the texture?
        this->m_texture = textureId;
        emit textureIdChanged(textureId);
        update();
    }

signals:
    void textureIdChanged(const uint& textureId);

protected:
    void updateTexture(QSGGeometryNode*& node);

    GLuint m_texture;
};

#endif // OMX_TEXTUREDELEMENT_H
