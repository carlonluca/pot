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

#ifndef OMX_OMXPLAYERCONTROLLER_H
#define OMX_OMXPLAYERCONTROLLER_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QObject>
#include <QProcess>
#include <QUrl>
#include <QMutex>
#include <QFileSystemWatcher>
#include <QThread>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRect>
#include <QWaitCondition>
#include <QDebug>
#include <QMediaPlayer>
#include <QStateMachine>

#include <functional>

#include "omx_logging.h"

template<typename T>
using POT_DbusCall = std::function<QDBusReply<T>(QDBusInterface*)>;

class QDBusPendingCallWatcher;
class OMX_OmxplayerController;

/*------------------------------------------------------------------------------
|    OMX_Process class
+-----------------------------------------------------------------------------*/
class OMX_Process : public QProcess {
public:
    OMX_Process() : QProcess() {}
    virtual ~OMX_Process() override;
};

/*------------------------------------------------------------------------------
|    OMX_GeometrySet class
+-----------------------------------------------------------------------------*/
class OMX_GeometryDispatcher : public QThread
{
    Q_OBJECT
public:
    OMX_GeometryDispatcher(OMX_OmxplayerController* controller) :
        QThread(),
        m_controller(controller) {}

    void dispatch();
    void run() override;
    void dispose();

signals:
    void sendGeometry();

private:
    OMX_OmxplayerController* m_controller;
    QWaitCondition m_cond;
    QMutex m_mutex;
    bool m_pending;
};

/*------------------------------------------------------------------------------
|    OMX_LastIn64 class
+-----------------------------------------------------------------------------*/
struct OMX_LastInt64
{
    qint64 val;
    qint64 msecs;
};

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController class
+-----------------------------------------------------------------------------*/
class OMX_OmxplayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QMediaPlayer::MediaStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QSize resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)
    Q_PROPERTY(bool frameVisible READ frameVisible NOTIFY frameVisibleChanged)
public:
    OMX_OmxplayerController(QObject* parent = nullptr);
    ~OMX_OmxplayerController();

    qint64 streamLength();
    qint64 streamPosition();
    qint64 width();

    static void prepareVideoLayer();

    QMediaPlayer::MediaStatus status() const { return m_status; }
    QSize resolution() const { return m_resolution; }
    bool frameVisible() const { return m_frameVisible; }

public slots:
    void play();
    bool stop();

    void streamLengthAsync();
    void streamPositionAsync();

    bool setFilename(QUrl url);
    bool setLayer(int layer);
    void setX(qreal x);
    void setY(qreal y);
    void setWidth(qreal w);
    void setHeight(qreal h);
    void setFillMode(Qt::AspectRatioMode mode) { m_fillMode = mode; }
    void setResolution(QSize resolution);

signals:
    void stopped();
    void streamLengthComputed(qint64 length);
    void streamPositionComputed(qint64 position);

    void loadRequested();
    void loadSucceeded();
    void loadFailed();
    void playRequested();
    void playSucceeded();

    void statusChanged(QMediaPlayer::MediaStatus status);

    void resolutionChanged(QSize resolution);
    void frameVisibleChanged(bool visible);

private slots:
    void updateDBusAddress();
    void updateDBusFilePath();
    void killProcess();
    void sendGeometry();

    void loadInternal();
    void playInternal();
    void connectIfNeeded();

    void eosReceived();
    void startReceived();

private:
    void setStatus(QMediaPlayer::MediaStatus status);
    void setFrameVisible(bool visible);
    bool isRunning();
    template<typename T> T dbusSend(
            QDBusInterface** iface,
            POT_DbusCall<T> command,
            T failure);
    void dbusSendAsync(
            const QString& command,
            std::function<void(QDBusPendingCallWatcher*)> lambda);
    QSize computeResolution(QUrl url);

    QDBusInterface* m_dbusIfaceProps;
    QDBusInterface* m_dbusIfacePlayer;
    QThread*  m_thread;
    QMutex    m_mutex;
    QMutex    m_geometryMutex;
    QProcess* m_process;
    QUrl      m_url;
    QString   m_dbusAddress;
    QString   m_dbusFilePath;
    QFileSystemWatcher* m_watcher;
    QRect m_rect;
    Qt::AspectRatioMode m_fillMode;

    OMX_LastInt64 m_lastDuration;
    OMX_LastInt64 m_lastPosition;
    OMX_GeometryDispatcher* m_dispatcher;
    QSize m_resolution;
    QMediaPlayer::MediaStatus m_status;

    QStateMachine* m_machine;
    bool m_frameVisible;

    friend class OMX_GeometryDispatcher;
};

#endif // OMX_OMXPLAYERCONTROLLER_H
