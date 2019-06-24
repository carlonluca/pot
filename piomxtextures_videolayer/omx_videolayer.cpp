/*
 * Project: PiOmxTexturesVideoLayer
 * Author:  Luca Carlon
 * Date:    01.02.2016
 *
 * Copyright (c) 2016 Luca Carlon. All rights reserved.
 *
 * This file is part of PiOmxTexturesVideoLayer.
 *
 * PiOmxTexturesVideoLayer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PiOmxTexturesVideoLayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PiOmxTexturesVideoLayer. If not, see <http://www.gnu.org/licenses/>.
 */

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QElapsedTimer>
#include <QTimer>
#include <QRectF>
#include <QColor>
#include <QSGSimpleRectNode>

#include "omx_videolayer.h"
#include "omx_logging.h"
#include "omx_globals.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#define ASYNC(...) \
    QMetaObject::invokeMethod(__VA_ARGS__)

/*------------------------------------------------------------------------------
|    class OMX_VideoRectNode
+-----------------------------------------------------------------------------*/
class OMX_VideoRectNode : public QSGSimpleRectNode
{
public:
    OMX_VideoRectNode() : QSGSimpleRectNode(), m_holeEnabled(true) {
        // Main material.
        QSGFlatColorMaterial* material = new QSGFlatColorMaterial;
        material->setColor(Qt::transparent);
        markDirty(QSGNode::DirtyMaterial);
        setMaterial(material);
        setFlag(OwnsMaterial);

        // Hole material.
        QSGFlatColorMaterial* materialHole  = new QSGFlatColorMaterial;
        materialHole->setColor(Qt::transparent);
        materialHole->setFlag(QSGMaterial::Blending, false);

        // Hole.
        m_hole = new QSGSimpleRectNode;
        m_hole->markDirty(QSGNode::DirtyMaterial);
        m_hole->setMaterial(materialHole);
        m_hole->setFlag(OwnsMaterial);

        appendChildNode(m_hole);
    }

    ~OMX_VideoRectNode() {
        delete m_hole;
    }

    void setHoleRect(const QRectF& rect) { m_hole->setRect(rect); }
    void enableHole() {
        if (m_holeEnabled)
            return;
        appendChildNode(m_hole);
        m_holeEnabled = true;
    }
    void disableHole() {
        if (!m_holeEnabled)
            return;
        removeChildNode(m_hole);
        m_holeEnabled = false;
    }

private:
    QSGSimpleRectNode* m_hole;
    bool m_holeEnabled;
};

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::OMX_VideoLayer
+-----------------------------------------------------------------------------*/
OMX_VideoLayer::OMX_VideoLayer(QQuickItem* parent) :
    QQuickItem(parent)
  , m_layer(0)
  , m_videoRect(boundingRect())
  , m_controller(new OMX_OmxplayerController)
  , m_fillMode(Qt::IgnoreAspectRatio)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setVlState(m_controller->status());

    connect(m_controller, SIGNAL(stopped()),
            this, SIGNAL(stopped()));
    connect(m_controller, SIGNAL(streamLengthComputed(qint64)),
            this, SIGNAL(durationReceived(qint64)));
    connect(m_controller, SIGNAL(streamPositionComputed(qint64)),
            this, SIGNAL(positionReceived(qint64)));
    connect(m_controller, SIGNAL(resolutionChanged(QSize)),
            this, SIGNAL(resolutionChanged(QSize)));
    connect(m_controller, SIGNAL(frameVisibleChanged(bool)),
            this, SIGNAL(videoFrameVisibleChanged(bool)));

    connect(this, SIGNAL(xChanged()),
            this, SLOT(onXChanged()), Qt::DirectConnection);
    connect(this, SIGNAL(yChanged()),
            this, SLOT(onYChanged()), Qt::DirectConnection);
    connect(this, SIGNAL(widthChanged()),
            this, SLOT(onWidthChanged()), Qt::DirectConnection);
    connect(this, SIGNAL(heightChanged()),
            this, SLOT(onHeightChanged()), Qt::DirectConnection);
    connect(m_controller, &OMX_OmxplayerController::statusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        setVlState(status);
        m_lastGlobalRect = QRectF();
        refreshContent();
    });
    connect(m_controller, &OMX_OmxplayerController::frameVisibleChanged, this, [this] {
        update();
    });

    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, [this] {
        refreshContent();
    });
    timer->start();

    m_controller->setFillMode(m_fillMode);
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::~OMX_VideoLayer
+-----------------------------------------------------------------------------*/
OMX_VideoLayer::~OMX_VideoLayer()
{
    m_controller->deleteLater();
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::updatePaintNode
+-----------------------------------------------------------------------------*/
QSGNode* OMX_VideoLayer::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    OMX_VideoRectNode* node = nullptr;
    if (!oldNode)
        // Create the node.
        node = new OMX_VideoRectNode;
    else
        node = static_cast<OMX_VideoRectNode*>(oldNode);

    if (m_controller->frameVisible())
        node->enableHole();
    else
        node->disableHole();

    node->setRect(boundingRect());
    node->setHoleRect(videoRect());

    return node;
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::layer
+-----------------------------------------------------------------------------*/
int OMX_VideoLayer::videoLayer()
{
    return m_layer;
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::setLayer
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::setVideoLayer(int layer)
{
    if (layer == m_layer)
        return;
    m_layer = layer;
    emit videoLayerChanged();
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::source
+-----------------------------------------------------------------------------*/
QUrl OMX_VideoLayer::source()
{
    return m_source;
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::setSource
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::setSource(QUrl source)
{
    if (m_source == source)
        return;
    log_debug_func;
    m_source = source;
    emit sourceChanged();

    m_controller->setFilename(source);
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::duration
+-----------------------------------------------------------------------------*/
qint64 OMX_VideoLayer::duration()
{
    return m_controller->streamLength();
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::position
+-----------------------------------------------------------------------------*/
qint64 OMX_VideoLayer::position()
{
    return m_controller->streamPosition();
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::play
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::play()
{
    ASYNC(m_controller, "play");
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::stop
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::stop()
{
    ASYNC(m_controller, "stop");
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::requestDuration
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::requestDuration()
{
    ASYNC(m_controller, "streamLengthAsync");
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::requestPosition
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::requestPosition()
{
    ASYNC(m_controller, "streamPositionAsync");
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::onXChanged
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::onXChanged()
{
    //m_controller.setX(x());
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::onYChanged
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::onYChanged()
{
    //m_controller.setY(y());
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::onWidthChanged
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::onWidthChanged()
{
    //m_controller.setWidth(width());
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::onHeightChanged
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::onHeightChanged()
{
    //m_controller.setHeight(height());
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::setFillMode
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::setFillMode(Qt::AspectRatioMode fillMode)
{
    if (m_fillMode == fillMode)
        return;
    m_fillMode = fillMode;
    refreshContent();
    emit fillModeChanged(m_fillMode);
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::setVlState
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::setVlState(QMediaPlayer::MediaStatus vlState)
{
    if (m_vlState == vlState)
        return;
    m_vlState = vlState;
    emit vlStateChanged(m_vlState);
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::geometryChanged
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    refreshContent();
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::refreshContent
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::refreshContent()
{
    if (vlState() != QMediaPlayer::LoadedMedia && vlState() != QMediaPlayer::BufferedMedia)
        return;

    QSize resolution = m_controller->resolution();
    if (resolution.isNull()) {
        log_warn("Failed to refresh content. No resolution");
        return; // TODO
    }

    QRectF full = boundingRect();
    QRectF videoRect = boundingRect();
    if (m_fillMode == Qt::KeepAspectRatio) {
        qreal r1 = full.width()/full.height();
        qreal r2 = qreal(resolution.width())/resolution.height();
        if (r2 >= r1) {
            qreal w = full.width();
            qreal h = w/r2;
            qreal x = 0;
            qreal y = (full.height() - h)/2;
            videoRect = QRectF(x, y, w, h);
        }
        else {
            qreal h = full.height();
            qreal w = h/r2;
            qreal y = 0;
            qreal x = (full.width() - w)/2;
            videoRect = QRectF(x, y, w, h);
        }
    }

    // If video rect remains unaltered, nothing else is needed.
    if (!setVideoRect(videoRect))
        return;
    refreshHwSurfaceGeometry();
    ASYNC(this, "update");
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::refreshHwSurfaceGeometry
+-----------------------------------------------------------------------------*/
void OMX_VideoLayer::refreshHwSurfaceGeometry()
{
    QRectF rect = videoRect();
    QPointF globalTopLeft = mapToScene(rect.topLeft());
    QRectF globalRect(globalTopLeft, rect.size());
    if (m_lastGlobalRect == globalRect)
        return;

    m_controller->setX(globalRect.x());
    m_controller->setY(globalRect.y());
    m_controller->setWidth(globalRect.width());
    m_controller->setHeight(globalRect.height());

    m_lastGlobalRect = globalRect;
}

/*------------------------------------------------------------------------------
|    OMX_VideoLayer::setVideoRect
+-----------------------------------------------------------------------------*/
bool OMX_VideoLayer::setVideoRect(const QRectF& rect)
{
    if (rect == m_videoRect)
        return false;
    m_videoRect = rect;
    emit videoRectChanged(rect);
    return true;
}
