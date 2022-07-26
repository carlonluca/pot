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
#include <QWaitCondition>
#include <QFileSystemWatcher>
#include <QThread>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRect>
#include <QWaitCondition>
#include <QDebug>
#include <QMediaPlayer>
#include <QStateMachine>
#include <QQueue>
#include <QTimer>
#include <QSemaphore>

#include <functional>
#include <lqtutils_threading.h>

#include "omx_logging_cat.h"
#include "../3rdparty/lqtutils/lqtutils_prop.h"

template<typename T>
using POT_DbusCall = std::function<QDBusReply<T>(QDBusInterface*)>;
using POT_DbusCallVoid = std::function<QDBusReply<void>(QDBusInterface*)>;

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
|    OMX_CommandProcessor class
+-----------------------------------------------------------------------------*/
class OMX_CommandProcessor : public QThread
{
    Q_OBJECT
public:
    enum CommandType {
        CMD_NONE,
        CMD_SET_SOURCE,
        CMD_PLAY,
        CMD_PAUSE,
        CMD_STOP,
        CMD_CLEAN_THREAD
    };
    Q_ENUM(CommandType)

    struct Command {
        Command(CommandType type, const QVariant& data) :
            type(type), data(data) {}
        Command(CommandType type) :
            type(type) {}
        Command() :
            type(CommandType::CMD_NONE) {}
        CommandType type;
        QVariant data;
    };

    OMX_CommandProcessor(OMX_OmxplayerController* controller) :
        QThread(), m_controller(controller), m_ready(0) {}
    void schedule(const Command& cmd);
    void run() override;
    void ready();

private:
    OMX_OmxplayerController* m_controller;
    QQueue<Command> m_pending;
    QMutex m_mutex;
    QMutex m_executionLock;
    QWaitCondition m_cond;
    QSemaphore m_ready;
};

#if QT_VERSION_MAJOR > 5
typedef QMediaPlayer::PlaybackState OMX_MediaPlayerState;
#else
typedef QMediaPlayer::State OMX_MediaPlayerState;
#endif

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController class
+-----------------------------------------------------------------------------*/
class OMX_OmxplayerController : public QObject
{
	Q_OBJECT
    Q_PROPERTY(QMediaPlayer::MediaStatus mediaStatus READ mediaStatus WRITE setMediaStatus NOTIFY mediaStatusChanged)
    Q_PROPERTY(OMX_MediaPlayerState playbackState READ playbackState WRITE setPlaybackState NOTIFY playbackStateChanged)
    Q_PROPERTY(QSize resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)
    Q_PROPERTY(bool frameVisible READ frameVisible NOTIFY frameVisibleChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    L_RW_PROP_AS(bool, loop, false)
    L_RW_PROP_AS(bool, waitForPlayAtBeginning, false)
    L_RW_PROP_AS(bool, waitForPlayAtEnd, false)
    L_RW_PROP_AS(bool, useAlsa, false)
    L_RW_PROP_AS(int, videoLayer, -128)
public:
	enum Orientation {
		ROT_0,
		ROT_90,
		ROT_180,
		ROT_270
	};
	Q_ENUM(Orientation)

    OMX_OmxplayerController(QObject* parent = nullptr);
    ~OMX_OmxplayerController();

    qint64 streamLength();
    qint64 streamPosition();
    qint64 width();

    static void prepareVideoLayer();

    QMediaPlayer::MediaStatus mediaStatus() const { return m_status; }
    OMX_MediaPlayerState playbackState() const { return m_state; }
    QSize resolution() const { return m_resolution; }
    bool frameVisible() const { return m_frameVisible; }
    bool muted() const { return m_muted; }
	void setOrientation(Orientation orientation) { m_orientation = orientation; }

public slots:
    void play(int position = 0);
    void stop();
    void pause();
    bool setPosition(qint64 microsecs);
    void setSource(const QUrl& url);

    void streamLengthAsync();
    void streamPositionAsync();

    void setX(qreal x);
    void setY(qreal y);
    void setWidth(qreal w);
    void setHeight(qreal h);
    void setFillMode(Qt::AspectRatioMode mode) { m_fillMode = mode; }
    void setResolution(QSize resolution);
    void setMuted(bool muted);

signals:
    void interrupted();
    void finished();
    void streamLengthComputed(qint64 length);
    void streamPositionComputed(qint64 position);

    void loadRequested();
    void loadSucceeded();
    void loadFailed();
    void playRequested();
    void playSucceeded();
    void pauseRequested();
    void stopRequested();

    void resolutionChanged(QSize resolution);
    void frameVisibleChanged(bool visible);

    void mediaStatusChanged(QMediaPlayer::MediaStatus mediaStatus);
    void playbackStateChanged(OMX_MediaPlayerState playbackState);

    void mutedChanged(bool muted);

    void eosReceived();
    void eosWaitingReceived();
    void startReceived();

private slots:
    void killProcess();
    void sendGeometry();

    void loadInternal();
    void playInternal(int position);
    void pauseInternal();
    void stopInternal();
    bool setFilenameInternal(QUrl url);

    void connectIfNeeded();

    void eosDbusReceived();
    void startDbusReceived();

private:
    void setMediaStatus(QMediaPlayer::MediaStatus mediaStatus);
    void setPlaybackState(OMX_MediaPlayerState playbackState);

    void setFrameVisible(bool visible);
    bool isRunning();
    template<typename T> T dbusSend(
            QDBusInterface* iface,
            POT_DbusCall<T> command,
            T failure);
    bool dbusSend(QDBusInterface* iface, POT_DbusCallVoid command);
    void dbusSendAsync(
            const QString& command,
            std::function<void(QDBusPendingCallWatcher*)> lambda);
    QSize computeResolution(QUrl url);

    void processCommand(const OMX_CommandProcessor::Command& cmd);
    bool isInStates(const QSet<QAbstractState*> states);
    void waitForStates(const QSet<QAbstractState*> states);
    QStringList readOmxplayerArguments();

    OMX_CommandProcessor* m_cmdProc;

    QScopedPointer<QDBusInterface> m_dbusIfaceProps;
    QScopedPointer<QDBusInterface> m_dbusIfacePlayer;
    QThread*  m_thread;
    LQTRecursiveMutex m_mutex;
    QMutex    m_geometryMutex;
    QProcess* m_process;
    QUrl      m_url;
    QString   m_dbusService;
    QRect m_rect;
    Qt::AspectRatioMode m_fillMode;

    OMX_LastInt64 m_lastDuration;
    OMX_LastInt64 m_lastPosition;
    OMX_GeometryDispatcher* m_dispatcher;
    QSize m_resolution;
    QMediaPlayer::MediaStatus m_status;

    QStateMachine* m_machine;
    bool m_frameVisible;

    OMX_MediaPlayerState m_state;
    bool m_muted;
    int m_vol;
    Orientation m_orientation;

    QState* m_stateNoMedia;
    QState* m_stateLoading;
    QState* m_stateLoaded;
    QState* m_statePlaying;
    QState* m_stateStopping;
    QState* m_statePaused;
    QState* m_stateEom;

    QTimer* m_dbusConnMonitor;

    OMX_CommandProcessor::Command m_lastPlayCommand;

    friend class OMX_GeometryDispatcher;
    friend class OMX_CommandProcessor;
};

#endif // OMX_OMXPLAYERCONTROLLER_H
