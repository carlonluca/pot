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
#include "lc_logging.h"


/*------------------------------------------------------------------------------
|    OMX_ImageElement::OMX_ImageElement
+-----------------------------------------------------------------------------*/
OMX_VideoSurfaceElement::OMX_VideoSurfaceElement(QQuickItem *parent)
    : QQuickItem(parent),
      m_source(NULL),
#ifdef ENABLE_VIDEO_PROCESSOR
      m_videoProc(NULL) // Cannot be init yet. EGL is not ready.
#elif ENABLE_MEDIA_PROCESSOR
      m_mediaProc(NULL), // Cannot be init yet. EGL is not ready.
#else
      m_mediaProc(NULL),
#endif
      m_sgtexture(new OMX_SGTexture(0, QSize(0, 0))),
      m_textureSize(QSize(0, 0)),
      m_textureId(0)
{
    setFlag(ItemHasContents, true);

    // TODO: Avoid updating when not needed.
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
    delete m_sgtexture;
}

/*------------------------------------------------------------------------------
|    OMX_ImageElement::updatePaintNode
+-----------------------------------------------------------------------------*/
QSGNode* OMX_VideoSurfaceElement::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    QSGGeometryNode* node = 0;
    QSGGeometry* geometry = 0;

#ifdef ENABLE_PROFILING
    static QElapsedTimer timer;
    LOG_DEBUG(LOG_TAG, "Elapsed: %lld.", timer.restart());
#endif // ENABLE_PROFILING

    if (!oldNode) {
        // Create the node.
        node = new QSGGeometryNode;
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
        geometry->setDrawingMode(GL_TRIANGLE_STRIP);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);

        // TODO: Who is freeing the material?
        QSGOpaqueTextureMaterial* material = new QSGOpaqueTextureMaterial;
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
        //geometry->allocate(4);

        // Texture has changed. Change the texture item for the scene graph.
        if ((GLuint)m_sgtexture->textureId() != m_textureId) {
            QMutexLocker locker(&m_mutexTexture);
            m_sgtexture->setTexture(m_textureId, m_textureSize);
        }

        // This is needed since Qt 5.1.1.
        node->markDirty(QSGNode::DirtyMaterial);
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
void OMX_VideoSurfaceElement::setSource(QObject* source)
{
    LOG_VERBOSE(LOG_TAG, "Source was set: %x.", (unsigned int)source);
    if (m_source)
        disconnect(m_source);

    // Reset the current texture.
    //onTextureChanged(0, QSize(0, 0));
    onTextureInvalidated();

    // Get notifications from the OMX_MediaProcessorElement related to this
    // OMX_VideoSurfaceElement of the texture I should use.
    m_source = (OMX_MediaProcessorElement*)source;
    connect(m_source, SIGNAL(textureReady(const OMX_TextureData*)),
            this, SLOT(onTextureChanged(const OMX_TextureData*)), Qt::DirectConnection);
    connect(m_source, SIGNAL(textureInvalidated()),
            this, SLOT(onTextureInvalidated()), Qt::DirectConnection);
    emit sourceChanged(source);
}

/*------------------------------------------------------------------------------
|    OMX_VideoSurfaceElement::onTextureChanged
+-----------------------------------------------------------------------------*/
void OMX_VideoSurfaceElement::onTextureChanged(const OMX_TextureData* textureData)
{
    LOG_INFORMATION(LOG_TAG, "Setting new texture %u!", textureData->m_textureId);

    QMutexLocker locker(&m_mutexTexture);
    m_textureId   = textureData->m_textureId;
    m_textureSize = textureData->m_textureSize;
}

/*------------------------------------------------------------------------------
|    OMX_VideoSurfaceElement::onTextureInvalidated
+-----------------------------------------------------------------------------*/
void OMX_VideoSurfaceElement::onTextureInvalidated()
{
    LOG_VERBOSE(LOG_TAG, "Texture was invalidated. Switching to 0.");

    QMutexLocker locker(&m_mutexTexture);
    m_textureId   = 0;
    m_textureSize = QSize(0, 0);
}
