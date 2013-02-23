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
#include <QSGTextureProvider>
#include <QtConcurrent/QtConcurrent>
#include <QQuickWindow>
#include <QSurface>

// Private headers.
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include "omx_videosurfaceelement.h"
#include "omx_texture.h"
#include "lgl_logging.h"


/*------------------------------------------------------------------------------
|    OMX_ImageElement::OMX_ImageElement
+-----------------------------------------------------------------------------*/
OMX_VideoSurfaceElement::OMX_VideoSurfaceElement(QQuickItem *parent)
    : QQuickItem(parent),
      m_source(NULL),
      m_texture(0),
#ifdef ENABLE_VIDEO_PROCESSOR
      m_videoProc(NULL) // Cannot be init yet. EGL is not ready.
#elif ENABLE_MEDIA_PROCESSOR
      m_mediaProc(NULL) // Cannot be init yet. EGL is not ready.
#else
      m_mediaProc(NULL)
#endif
{
    setFlag(ItemHasContents, true);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(40);
}

/*------------------------------------------------------------------------------
|    OMX_ImageElement::~OMX_ImageElement
+-----------------------------------------------------------------------------*/
OMX_VideoSurfaceElement::~OMX_VideoSurfaceElement()
{
    // Do nothing.
}

/*------------------------------------------------------------------------------
|    OMX_ImageElement::updatePaintNode
+-----------------------------------------------------------------------------*/
QSGNode* OMX_VideoSurfaceElement::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
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
        // TODO: I cannot know the texture size here.
        QSGOpaqueTextureMaterial* material = NULL;
        if (m_source && m_source->mediaProcessor()) {
            int width   = m_source->mediaProcessor()->m_hints_video.width;
            int height  = m_source->mediaProcessor()->m_hints_video.height;
            m_texture   = m_source->mediaProcessor()->textureId();
            m_sgtexture = new OMX_SGTexture(m_texture, QSize(width, height));
        }
        else
            m_sgtexture = new OMX_SGTexture(m_texture, QSize(0, 0));

        material = new QSGOpaqueTextureMaterial;
        material->setTexture(m_sgtexture);
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);

#ifdef ENABLE_VIDEO_PROCESSOR
        QPlatformNativeInterface* nativeInterface =
                QGuiApplicationPrivate::platformIntegration()->nativeInterface();
        Q_ASSERT(nativeInterface);
        EGLDisplay eglDisplay = nativeInterface->nativeResourceForIntegration("egldisplay");
        EGLContext eglContext = nativeInterface->nativeResourceForContext(
                    "eglcontext",
                    QOpenGLContext::currentContext()
                    );
#endif
        // Provider MUST be built in this thread.
#ifdef ENABLE_VIDEO_PROCESSOR
        m_videoProc = new OMX_VideoProcessor(eglDisplay, eglContext, m_provider);
        connect(m_videoProc, SIGNAL(textureReady(uint)), this, SLOT(onTextureChanged(uint)));
        if (!m_source.isNull())
            m_videoProc->setVideoPath(m_source);
        if (m_playScheduled) {
            m_timer->start(30);
            m_videoProc->play();
        }
#elif ENABLE_MEDIA_PROCESSOR
#else
        LOG_VERBOSE(LOG_TAG, "Starting video...");
        QtConcurrent::run(&startVideo, m_provider, this);
        m_timer->start(30);
#endif
    }
    else {
        node = static_cast<QSGGeometryNode*>(oldNode);
        geometry = node->geometry();
        geometry->allocate(4);

        // Update texture in the node if needed.
        QSGOpaqueTextureMaterial* material = (QSGOpaqueTextureMaterial*)node->material();
        if (material && material->texture() && m_texture != (GLuint)material->texture()->textureId()) {
            // TODO: Does setTextureId frees the prev texture?
            // TODO: I should the given the texture size.
            LOG_ERROR(LOG_TAG, "Updating texture to %u!", m_texture);
            material = new QSGOpaqueTextureMaterial;
            m_sgtexture->setTexture(m_texture, QSize(854, 480));
        }
    }

    // Create the vertices and map to texture.
    QRectF bounds = boundingRect();
    QSGGeometry::TexturedPoint2D* vertices = geometry->vertexDataAsTexturedPoint2D();
    vertices[0].set(bounds.x(), bounds.y() + bounds.height(), 0.0f, 0.0f);
    vertices[1].set(bounds.x() + bounds.width(), bounds.y() + bounds.height(), 1.0f, 0.0f);
    vertices[2].set(bounds.x(), bounds.y(), 0.0f, 1.0f);
    vertices[3].set(bounds.x() + bounds.width(), bounds.y(), 1.0f, 1.0f);
    return node;
}

/*------------------------------------------------------------------------------
|    OMX_VideoSurfaceElement::onTextureChanged
+-----------------------------------------------------------------------------*/
void OMX_VideoSurfaceElement::onTextureChanged(const GLuint &textureId)
{
    LOG_INFORMATION(LOG_TAG, "Setting new texture!");
    m_texture = textureId;
}
