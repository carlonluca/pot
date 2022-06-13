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

#ifndef OMX_VIDEOLAYER_H
#define OMX_VIDEOLAYER_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QObject>
#include <QQuickItem>
#include <QUrl>
#include <Qt>
#include <QMediaPlayer>

#include "omx_omxplayercontroller.h"
#include "omx_logging_cat.h"
#include "../3rdparty/lqtutils/lqtutils_prop.h"

/*------------------------------------------------------------------------------
|    OMX_VideoLayer class
+-----------------------------------------------------------------------------*/
class OMX_VideoLayer : public QQuickItem
{
    Q_OBJECT
public:
	enum Orientation {
		ROT_0,
		ROT_90,
		ROT_180,
		ROT_270
	};
	Q_ENUM(Orientation)

private:
    Q_PROPERTY(int videoLayer READ videoLayer WRITE setVideoLayer NOTIFY videoLayerChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(qint64 duration READ duration)
    Q_PROPERTY(qint64 position READ position)
    Q_PROPERTY(Qt::AspectRatioMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    Q_PROPERTY(QMediaPlayer::MediaStatus vlState READ vlState WRITE setVlState NOTIFY vlStateChanged)
    Q_PROPERTY(QSize resolution READ resolution NOTIFY resolutionChanged)
    Q_PROPERTY(QRectF videoRect READ videoRect NOTIFY videoRectChanged)
    Q_PROPERTY(bool videoFrameVisible READ videoFrameVisible NOTIFY videoFrameVisibleChanged)
    Q_PROPERTY(bool autoPlay READ autoPlay WRITE setAutoPlay NOTIFY autoPlayChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    L_RO_PROP_AS(OMX_MediaPlayerState, playbackState, OMX_MediaPlayerState::StoppedState)
    L_RW_PROP_AS(bool, loop, false)

public:
    OMX_VideoLayer(QQuickItem* parent = nullptr);
    ~OMX_VideoLayer() override;

    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

    int videoLayer();
    void setVideoLayer(int videoLayer);

    QUrl source();
    virtual void setSource(QUrl source);

    qint64 duration();
    qint64 position();

    Qt::AspectRatioMode fillMode() const { return m_fillMode; }
    QMediaPlayer::MediaStatus vlState() const { return m_vlState; }
    QSize resolution() const { return m_controller->resolution(); }
    QRectF videoRect() const { return m_videoRect; }
    bool videoFrameVisible() const { return m_controller->frameVisible(); }
    bool autoPlay() const { return m_autoPlay; }
    bool muted() const { return m_controller->muted(); }
	Orientation orientation() const { return m_orientation; }

	void setOrientation(Orientation orientation);

public slots:
    void play(int position = 0);
    void stop();
    void pause();
    void seek(qint64 micros);
    void resume();

    void requestDuration();
    void requestPosition();

    void onXChanged();
    void onYChanged();
    void onWidthChanged();
    void onHeightChanged();

    void setFillMode(Qt::AspectRatioMode fillMode);
    void setVlState(QMediaPlayer::MediaStatus vlState);
    void setAutoPlay(bool autoPlay);
    void setMuted(bool muted);

protected:
#if QT_VERSION_MAJOR > 5
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#else
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#endif
	bool setVideoRect(const QRectF& rect);
	QRectF computeGlobalRect(const QRectF& localRect);

signals:
    void videoLayerChanged();
    void sourceChanged();
    void stopped();
    void durationReceived(qint64 d);
    void positionReceived(qint64 p);
    void fillModeChanged(Qt::AspectRatioMode fillMode);
    void vlStateChanged(QMediaPlayer::MediaStatus vlState);
    void resolutionChanged(QSize resolution);
    void videoRectChanged(QRectF videoRect);
    void videoFrameVisibleChanged(bool videoFrameVisible);
    void autoPlayChanged(bool autoPlay);
    void mutedChanged(bool muted);
	void orientationChanged();

protected slots:
    void refreshContent();
    void refreshHwSurfaceGeometry();

protected:
    int m_layer;
    QUrl m_source;
    QRectF m_lastGlobalRect;
    QRectF m_videoRect;

    OMX_OmxplayerController* m_controller;
    Qt::AspectRatioMode m_fillMode;
    QMediaPlayer::MediaStatus m_vlState;
    bool m_autoPlay;
	Orientation m_orientation;
};

#endif
