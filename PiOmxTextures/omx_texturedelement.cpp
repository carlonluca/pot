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

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QSGOpaqueTextureMaterial>
#include <QQuickWindow>

#include "omx_texturedelement.h"


/*------------------------------------------------------------------------------
|    OMX_TextureElement::OMX_TexturedElement
+-----------------------------------------------------------------------------*/
OMX_TexturedElement::OMX_TexturedElement(QQuickItem* parent) :
    QQuickItem(parent),
    m_texture(0)
{
    // Do nothing.
}

OMX_TexturedElement::~OMX_TexturedElement()
{
    // TODO: Do I have to dealloc something?
}

/*------------------------------------------------------------------------------
|    OMX_TexturedElement::updatePaintNode
+-----------------------------------------------------------------------------*/
QSGNode* OMX_TexturedElement::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    QSGGeometryNode* node = 0;
    QSGGeometry* geometry = 0;

    if (!oldNode) {
        // Create the node.
        node = new QSGGeometryNode;
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
        geometry->setDrawingMode(GL_TRIANGLE_STRIP);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);

        // TODO: Who is freeing this?
        // TODO: Who is freeing the texture?
        QSGOpaqueTextureMaterial* material = new QSGOpaqueTextureMaterial;
        QSGTexture* mySGTexture = window()->createTextureFromId(m_texture, QSize(1920, 1080));
        material->setTexture(mySGTexture);
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
    }
    else {
        node = static_cast<QSGGeometryNode*>(oldNode);
        geometry = node->geometry();
        geometry->allocate(4);
        updateTexture(node);
    }

    // Create the vertices and map to texture.
    QRectF bounds = boundingRect();
    QSGGeometry::TexturedPoint2D* vertices = geometry->vertexDataAsTexturedPoint2D();
    vertices[0].set(bounds.x(), bounds.y() + bounds.height(), 0.0f, 1.0f);
    vertices[1].set(bounds.x() + bounds.width(), bounds.y() + bounds.height(), 1.0f, 1.0f);
    vertices[2].set(bounds.x(), bounds.y(), 0.0f, 0.0f);
    vertices[3].set(bounds.x() + bounds.width(), bounds.y(), 1.0f, 0.0f);
    return node;
}

/*------------------------------------------------------------------------------
|    OMX_TexturedElement::updateTexture
+-----------------------------------------------------------------------------*/
inline
void OMX_TexturedElement::updateTexture(QSGGeometryNode*& node)
{
    // Update texture in the node if needed.
    QSGOpaqueTextureMaterial* material = (QSGOpaqueTextureMaterial*)node->material();
    if (m_texture != (GLuint)material->texture()->textureId()) {
        // TODO: Does setTextureId frees the prev texture?
        // TODO: The size must be determined and passed from outside.
        QSGTexture* mySGTexture = window()->createTextureFromId(m_texture, QSize(1920, 1080));
        material->setTexture(mySGTexture);
    }
}
