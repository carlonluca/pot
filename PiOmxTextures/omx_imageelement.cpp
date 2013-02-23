/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.16.2012
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
#include <QSGNode>

// Private headers.
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include "omx_imageelement.h"
#include "omx_texture.h"
#include "lgl_logging.h"
#include "openmaxiltextureloader.h"


/*------------------------------------------------------------------------------
|    OMX_ImageElement::OMX_ImageElement
+-----------------------------------------------------------------------------*/
OMX_ImageElement::OMX_ImageElement(QQuickItem *parent)
    : QQuickItem(parent),
      m_source(""),
      m_texture(0)
{
    setFlag(ItemHasContents, true);
}

/*------------------------------------------------------------------------------
|    OMX_ImageElement::~OMX_ImageElement
+-----------------------------------------------------------------------------*/
OMX_ImageElement::~OMX_ImageElement()
{
}

/*------------------------------------------------------------------------------
|    OMX_ImageElement::updatePaintNode
+-----------------------------------------------------------------------------*/
QSGNode* OMX_ImageElement::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
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

        // Load and set the texture. The first time it is always needed to do this.
        updateTexture();

        // TODO: Who is freeing this?
        QSGOpaqueTextureMaterial* material = new QSGOpaqueTextureMaterial;
        material->setTexture(new OMX_SGTexture(m_texture, QSize(1920, 1080)));
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
    }
    else {
        node = static_cast<QSGGeometryNode *>(oldNode);
        geometry = node->geometry();
        geometry->allocate(4);

        // Update texture in the node if needed.
        QSGOpaqueTextureMaterial* material = (QSGOpaqueTextureMaterial*)node->material();
        if (m_texture != (GLuint)material->texture()->textureId()) {
            updateTexture();

            // TODO: Does setTextureId frees the prev texture?
            OMX_SGTexture* mySGTexture = (OMX_SGTexture*)material->texture();
            mySGTexture->setTexture(m_texture, QSize(1920, 1080));
        }
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
|    OMX_ImageElement::updateTexture
+-----------------------------------------------------------------------------*/
inline
void OMX_ImageElement::updateTexture()
{
    // Load the image.
    QPlatformNativeInterface* nativeInterface =
            QGuiApplicationPrivate::platformIntegration()->nativeInterface();
    Q_ASSERT(nativeInterface);
    EGLDisplay eglDisplay = nativeInterface->nativeResourceForIntegration("egldisplay");
    EGLContext eglContext = nativeInterface->nativeResourceForContext(
                "eglcontext",
                QOpenGLContext::currentContext()
                );

    // TODO: Who is freeing the loader?
    OpenMAXILTextureLoader* loader = OpenMAXILTextureLoader::intance();
    if (!QFile(m_source).exists())
        return;
    for (int i = 0; i < 1000; i++) {
        if (!loader->loadTextureFromImage(m_source, eglDisplay, eglContext, m_texture)) {
            LOG_ERROR(LOG_TAG, "Failed to load image.");
            return;
        }
        glDeleteTextures(1, &m_texture);
    }
    loader->freeInstance();
    LOG_INFORMATION(LOG_TAG, "Image successfully loaded.");
}
