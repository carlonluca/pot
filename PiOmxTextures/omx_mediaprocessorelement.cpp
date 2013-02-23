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

#include "omx_mediaprocessorelement.h"
#include "omx_textureproviderqquickitem.h"
#include "lgl_logging.h"

#define CHECK_MEDIA_PROCESSOR                                              \
    if (!m_mediaProc) {                                                    \
        LOG_WARNING(LOG_TAG, "The media processor is not available yet."); \
        return false;                                                      \
    }

OMX_MediaProcessorElement::OMX_MediaProcessorElement(QQuickItem *parent) :
    QQuickItem(parent),
    m_mediaProc(NULL),
    m_texProvider(NULL),
    m_textureId(0)
{
    // I need to set this as a "has-conent" item because I need the updatePaintNode
    // to be invoked.
    setFlag(QQuickItem::ItemHasContents, true);
}

OMX_MediaProcessorElement::~OMX_MediaProcessorElement()
{
    delete m_mediaProc;
    delete m_texProvider;
}

QString OMX_MediaProcessorElement::source()
{
    return m_source;
}

void OMX_MediaProcessorElement::setSource(QString source)
{
    LOG_VERBOSE(LOG_TAG, "Setting source.");

    // TODO: Handle errors.
    m_source = source;
    if (m_mediaProc) {
        if (openMedia(source))
            m_mediaProc->play();
        else {
            LOG_WARNING(LOG_TAG, "Failed to open media.");
        }
    }
    else {
        LOG_VERBOSE(LOG_TAG, "Play delayed.");
    }
}

QSGNode* OMX_MediaProcessorElement::updatePaintNode(QSGNode*, UpdatePaintNodeData*)
{
    if (!m_texProvider) {
        m_texProvider = new OMX_TextureProviderQQuickItem(this);
        m_mediaProc   = new OMX_MediaProcessor(m_texProvider);
        connect(m_mediaProc, SIGNAL(playbackCompleted()), this, SIGNAL(playbackCompleted()));
        connect(m_mediaProc, SIGNAL(playbackStarted()), this, SIGNAL(playbackStarted()));

        // Open if filepath is set.
        // TODO: Handle errors.
        if (!m_source.isNull()) {
            //if (QFile(m_source).exists()) {
                if (openMedia(m_source))
                    m_mediaProc->play();
            //}
            //else {
                LOG_WARNING(LOG_TAG, "File does not exist.");
            //}
        }
    }

    return NULL;

#if 0
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
        QSGOpaqueTextureMaterial* material = new QSGOpaqueTextureMaterial;
        m_sgtexture = new OMX_SGTexture(m_texture, QSize(1920, 1080));
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
        m_provider  = new OMX_TextureProviderQQuickItem(this);
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
        LOG_VERBOSE(LOG_TAG, "Starting video using media processor...");
        m_mediaProc = new OMX_MediaProcessor(m_provider);
        m_mediaProc->setFilename("/home/pi/usb/Cars2.mkv", m_texture);
        //if (m_playScheduled) {
            m_timer->start(40);
            m_mediaProc->play();
        //}
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
        if (m_texture != (GLuint)material->texture()->textureId()) {
            // TODO: Does setTextureId frees the prev texture?
            // TODO: I should the given the texture size.
            LOG_ERROR(LOG_TAG, "Updating texture to %u!", m_texture);
            material = new QSGOpaqueTextureMaterial;
            m_sgtexture->setTexture(m_texture, QSize(1920, 1080));
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
#endif
}

bool OMX_MediaProcessorElement::play()
{
    CHECK_MEDIA_PROCESSOR;
    return m_mediaProc->play();
}

bool OMX_MediaProcessorElement::stop()
{
    CHECK_MEDIA_PROCESSOR;
    return m_mediaProc->stop();
}

bool OMX_MediaProcessorElement::pause()
{
    CHECK_MEDIA_PROCESSOR;
    return m_mediaProc->pause();
}

bool OMX_MediaProcessorElement::seek(long millis)
{
    CHECK_MEDIA_PROCESSOR;
    return m_mediaProc->seek(millis);
}

long OMX_MediaProcessorElement::currentPosition()
{
    CHECK_MEDIA_PROCESSOR;
    return m_mediaProc->currentPosition();
}

bool OMX_MediaProcessorElement::openMedia(QString filepath)
{
    if (!m_mediaProc || !m_texProvider)
        return false;

    if (!m_mediaProc->setFilename(filepath, m_textureId))
        return false;
    emit textureReady(m_textureId);

    return true;
}
