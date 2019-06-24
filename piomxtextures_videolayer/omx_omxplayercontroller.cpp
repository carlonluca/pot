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
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include <QDateTime>
#include <QStringList>
#include <QGuiApplication>
#include <QElapsedTimer>
#include <QScreen>
#include <QDataStream>
#include <QDebug>
#include <QtMath>
#include <QRegularExpression>
#include <QStateMachine>
#include <QTimer>

#include <mutex>
#include <functional>
#include <signal.h>

#include <sys/xattr.h>

#include "omx_omxplayercontroller.h"
#include "omx_logging.h"
#include "omx_globals.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#define NO_CONTENT(u) \
    (u.isEmpty() || u.isNull())

#define PLAYER_COMMAND \
    "omxplayer"

#define USE_SIG_HANDLERS

static std::once_flag g_prepare;

/*------------------------------------------------------------------------------
|    POT_TermColor enum
+-----------------------------------------------------------------------------*/
enum POT_TermColor {
    T_C_BLACK,
    T_C_WHITE
};

/*------------------------------------------------------------------------------
|    isConnected
+-----------------------------------------------------------------------------*/
inline bool is_connected(QDBusInterface* iface)
{
    return (iface && iface->isValid() && iface->connection().isConnected());
}

/*------------------------------------------------------------------------------
|    prepare_dispmanx
+-----------------------------------------------------------------------------*/
#if 0
static void prepare_dispmanx()
{
    // Prepare layers to workaround dispmanx bug.
    const QStringList args = QStringList()
            << "--version";

    QProcess proc;
    proc.start(PLAYER_COMMAND, args);

#define COMMAND_OMX_TIMEOUT_MS 10000

    if (!proc.waitForFinished(COMMAND_OMX_TIMEOUT_MS)) {
        log_err("Timeout starting omx. Maybe GPU is hanging?");
        abort();
    }

    if (proc.exitCode()) {
        log_err("omx returned %d. Failed to init.", proc.exitCode());
        abort();
    }
}
#endif

/*------------------------------------------------------------------------------
|    setterm
+-----------------------------------------------------------------------------*/
static void setterm(POT_TermColor color)
{
    const char colorHex = color == T_C_BLACK ? 0x30 : 0x37;
    const char command[12] = {
        0x1b, 0x5b, 0x33, colorHex,
        0x6d, 0x1b, 0x5b, 0x48,
        0x1b, 0x5b, 0x32, 0x4a
    };


    QFile ttyDev("/dev/tty0");
    if (!ttyDev.open(QIODevice::WriteOnly)) {
        log_err("Failed to open tty.");
        return;
    }

    QDataStream out(&ttyDev);
    out.writeBytes(command, sizeof(command));
}

#ifdef USE_SIG_HANDLERS
typedef void (*sighandler)(int);

static QMap<int, sighandler> g_handlers;

/*------------------------------------------------------------------------------
-|    killall
-+-----------------------------------------------------------------------------*/
static void killall(const QString& procname)
{
    QProcess p;
    p.start("killall", QStringList() << "-9" << procname);
    p.waitForFinished(1000);
}

/*------------------------------------------------------------------------------
|    kill_omxplyer
+-----------------------------------------------------------------------------*/
static void kill_omxplayer()
{
    killall("omxplayer");
    killall("omxplayer.bin");
}

/*------------------------------------------------------------------------------
|    handle_death
+-----------------------------------------------------------------------------*/
static void handle_death()
{
    setterm(T_C_WHITE);

    kill_omxplayer();

    log_info("Bye bye ;-)");
}

/*------------------------------------------------------------------------------
|    sig_handler
+-----------------------------------------------------------------------------*/
static std::once_flag g_handler;

static void sig_handler(int s)
{
    Q_UNUSED(s);

    std::call_once(g_handler, [s] {
        log_info("Received signal %d - %s", s, strsignal(s));
        log_stacktrace(LOG_TAG, LC_LogLevel::LC_LOG_INFO, 1000);
        log_info("Cleaning up...");
        handle_death();
        if (s)
            qApp->quit();
    });
}
#endif // USE_SIG_HANDLERS

/*------------------------------------------------------------------------------
|    install_handlers
+-----------------------------------------------------------------------------*/
#ifdef USE_SIG_HANDLERS
static void install_handlers()
{
    // Using signal handlers in a plugin is not a good idea.

#if 0
#define REPLACE_SIGNAL_HANDLER(s, h)            \
    {                                            \
    struct sigaction current;                 \
    sigaction(s, NULL, &current);             \
    g_handlers.insert(s, current.sa_handler); \
    memset(&current, 0, sizeof(current));     \
    current.sa_handler = h;                   \
    sigaction(s, &current, NULL);             \
}
#endif
#define REPLACE_SIGNAL_HANDLER(s, h) \
    signal(s, sig_handler);

    log_debug("Installing sighandler...");
    REPLACE_SIGNAL_HANDLER(SIGINT, sig_handler)
            REPLACE_SIGNAL_HANDLER(SIGKILL, sig_handler)
            REPLACE_SIGNAL_HANDLER(SIGTERM, sig_handler)
            REPLACE_SIGNAL_HANDLER(SIGSEGV, sig_handler)
            REPLACE_SIGNAL_HANDLER(SIGFPE, sig_handler)
            REPLACE_SIGNAL_HANDLER(SIGABRT, sig_handler)
}
#endif // USE_SIG_HANDLERS

/*------------------------------------------------------------------------------
|    prepare_terminal
+-----------------------------------------------------------------------------*/
static void prepare_terminal()
{
#ifdef USE_SIG_HANDLERS
    install_handlers();
#endif // USE_SIG_HANDLERS

    //kill_omxplayer();

    //QObject::connect(qApp, &QCoreApplication::aboutToQuit, [] {
    //    sig_handler(0);
    //});

    setterm(T_C_BLACK);

#ifndef USE_SIG_HANDLERS
    const QString processName =
            QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    const QStringList args =
            QStringList() << QString("%1").arg(processName);
    const QString vtRestore =
            QStringLiteral("/tmp/restore.sh");

    {
        QFile file(":/omx_vt_restore.sh");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            abort();
        file.copy(vtRestore);
    }

    {
        QFile file(vtRestore);
        file.setPermissions(QFile::ExeUser | QFile::ReadUser);
    }

    QProcess::startDetached(vtRestore, args);
#endif // USE_SIG_HANDLERS
}

/*------------------------------------------------------------------------------
|    OMX_GeometryDispatcher::dispatch
+-----------------------------------------------------------------------------*/
void OMX_GeometryDispatcher::dispatch()
{
    m_mutex.lock();
    m_pending = true;
    m_cond.wakeOne();
    m_mutex.unlock();
}

/*------------------------------------------------------------------------------
|    OMX_GeometryDispatcher::run
+-----------------------------------------------------------------------------*/
void OMX_GeometryDispatcher::run()
{
    while (!isInterruptionRequested()) {
        m_mutex.lock();
        if (!m_pending)
            m_cond.wait(&m_mutex);
        m_pending = false;
        m_mutex.unlock();

        // Semaphore was signaled to allow to finish.
        if (isInterruptionRequested())
            return;

        m_controller->sendGeometry();
    }
}

/*------------------------------------------------------------------------------
|    OMX_GeometryDispatcher::dispose
+-----------------------------------------------------------------------------*/
void OMX_GeometryDispatcher::dispose()
{
    connect(this, SIGNAL(finished()),
            this, SLOT(deleteLater()));
    requestInterruption();
    dispatch();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::OMX_OmxplayerController
+-----------------------------------------------------------------------------*/
OMX_OmxplayerController::OMX_OmxplayerController(QObject* parent) :
    QObject(parent)
  , m_dbusIfaceProps(nullptr)
  , m_dbusIfacePlayer(nullptr)
  , m_thread(new QThread)
  , m_mutex(QMutex::Recursive)
  , m_rect(QPoint(0, 0), QGuiApplication::primaryScreen()->size())
  , m_fillMode(Qt::IgnoreAspectRatio)
  , m_lastDuration({-1, -1})
  , m_lastPosition({-1, -1})
  , m_status(QMediaPlayer::NoMedia)
  , m_frameVisible(false)
{
    prepareVideoLayer();

    connect(this, SIGNAL(destroyed()),
            m_thread, SLOT(quit()));
    connect(m_thread, SIGNAL(finished()),
            m_thread, SLOT(deleteLater()));

    m_process = new QProcess;

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this] {
        log_verbose("omx process closed");
        this->setFrameVisible(false);
    });

    updateDBusFilePath();

    m_watcher = new QFileSystemWatcher(QStringList() << m_dbusFilePath, this);
    connect(m_watcher, SIGNAL(fileChanged(QString)),
            this, SLOT(updateDBusAddress()));

    updateDBusAddress();

    //connect(m_process, SIGNAL(finished(int)),
    //        this, SIGNAL(stopped()));
    //connect(qApp, SIGNAL(aboutToQuit()),
    //        this, SLOT(killProcess()));

    // Start the dispatcher.
    m_dispatcher = new OMX_GeometryDispatcher(this);
    m_dispatcher->start();

    connect(m_dispatcher, SIGNAL(sendGeometry()),
            this, SLOT(sendGeometry()));
    connect(this, SIGNAL(destroyed(QObject*)),
            m_dispatcher, SLOT(quit()));
    connect(m_dispatcher, SIGNAL(finished()),
            m_dispatcher, SLOT(deleteLater()));

    QState* stateNoMedia = new QState;
    QState* stateLoading = new QState;
    QState* stateLoaded = new QState;
    QState* statePlaying = new QState;
    QState* stateEom = new QState;

    // No media.
    connect(stateNoMedia, &QState::entered, [this] {
        log_verbose("State entered: STATE_NO_MEDIA");
        setStatus(QMediaPlayer::NoMedia);
    });
    stateNoMedia->addTransition(this, SIGNAL(loadRequested()), stateLoading);

    // Loading.
    connect(stateLoading, &QState::entered, [this] {
        log_verbose("State entered: STATE_LOADING");
        setStatus(QMediaPlayer::LoadingMedia);
        loadInternal();
    });
    stateLoading->addTransition(this, SIGNAL(loadSucceeded()), stateLoaded);
    stateLoading->addTransition(this, SIGNAL(loadFailed()), stateNoMedia);
    // TODO: Handle failure here.

    // Loaded.
    connect(stateLoaded, &QState::entered, [this] {
        log_verbose("State entered: STATE_LOADED");
        setStatus(QMediaPlayer::LoadedMedia);
    });
    stateLoaded->addTransition(this, SIGNAL(playRequested()), statePlaying);

    // Playing.
    connect(statePlaying, &QState::entered, [this] {
        log_verbose("State entered: PLAYING");
        setStatus(QMediaPlayer::BufferedMedia);
        playInternal();
    });
    statePlaying->addTransition(this, SIGNAL(stopped()), stateEom);

    // EOM.
    connect(stateEom, &QState::entered, [this] {
        log_verbose("State entered: EOM");
        setStatus(QMediaPlayer::EndOfMedia);
    });
    stateEom->addTransition(this, SIGNAL(loadRequested()), stateLoading);
    stateEom->addTransition(this, SIGNAL(playRequested()), statePlaying);

    m_machine = new QStateMachine(this);
    m_machine->addState(stateNoMedia);
    m_machine->addState(stateLoading);
    m_machine->addState(stateLoaded);
    m_machine->addState(statePlaying);
    m_machine->addState(stateEom);
    m_machine->setInitialState(stateNoMedia);
    m_machine->start();

#ifdef USE_SIG_HANDLERS
    install_handlers();
#endif // USE_SIG_HANDLERS

    m_process->moveToThread(m_thread);
    moveToThread(m_thread);
    m_thread->start();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::~OMX_OmxplayerController
+-----------------------------------------------------------------------------*/
OMX_OmxplayerController::~OMX_OmxplayerController()
{
    QMutexLocker l(&m_mutex);

    // Kill process and free.
    if (m_process->state() == QProcess::NotRunning)
        delete m_process;
    else {
        //disconnect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)));
        //disconnect(m_process, SIGNAL(finished(int)));
        disconnect(m_process, nullptr, nullptr, nullptr);
        killProcess();
        delete m_process;
    }

    m_machine->stop();

    delete m_dbusIfaceProps;
    delete m_dbusIfacePlayer;

    m_dispatcher->dispose();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setFilename
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::setFilename(QUrl url)
{
    QMutexLocker locker(&m_mutex);
    if (url.isEmpty() || !url.isValid()) {
        m_url = QUrl();
        emit loadFailed();
        return true;
    }

    if (url.isLocalFile()) {
        if (!QFile(url.path()).exists()) {
            m_url = QUrl();
            log_warn("Media does not exist: %s", qPrintable(url.toString()));
            emit loadFailed();
            return false;
        }
    }

    m_url = url;

    // This is used to give time to the QStateMachine to init.
    QTimer::singleShot(0, this, [this] {
        emit loadRequested();
    });

    return true;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setLayer
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::setLayer(int layer)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(layer);

    // TODO: Implement.

    return false;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::play
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::play()
{
    emit playRequested();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::stop
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::stop()
{
    QMutexLocker locker(&m_mutex);

    if (m_process->state() == QProcess::Running) {
        log_verbose("Stopping...");
        killProcess();
    }

    return true;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::streamLength
+-----------------------------------------------------------------------------*/
qint64 OMX_OmxplayerController::streamLength()
{
    QMutexLocker locker(&m_mutex);
#if 0
    qlonglong v;
    STOPWATCH_THIS("dbusSend for Duration", {
                       if (!isRunning())
                       return 0;
                       v = dbusSend<qlonglong>("Duration", 0)/1000;
                   });
    return v;
#else
    const qint64 curr = QDateTime::currentMSecsSinceEpoch();
    const qint64 prev = m_lastDuration.msecs;
    if (LIKELY(prev != -1))
        if (curr - prev < 100)
            return m_lastDuration.val;

    if (!isRunning())
        return 0;

    const POT_DbusCall<qlonglong> f = [] (QDBusInterface* iface) -> QDBusReply<qlonglong> {
        return iface->call("Duration");
    };
    const qint64 duration = dbusSend<qlonglong>(&m_dbusIfaceProps, f, 0)/1000;
    m_lastDuration.msecs = QDateTime::currentMSecsSinceEpoch();
    m_lastDuration.val = duration;

    return duration;
#endif
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::streamLengthAsync
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::streamLengthAsync()
{
#if 0
    dbusSendAsync("Duration", [this](QDBusPendingCallWatcher* w) {
        const QDBusPendingReply<qint64> reply = *w;
        const qint64 length = reply.value()/1000;
        delete w;

        emit streamLengthComputed(length);
    });
#endif

    const qint64 length = streamLength();
    emit streamLengthComputed(length);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::streamPosition
+-----------------------------------------------------------------------------*/
qint64 OMX_OmxplayerController::streamPosition()
{
    QMutexLocker locker(&m_mutex);

    const qint64 curr = QDateTime::currentMSecsSinceEpoch();
    const qint64 prev = m_lastPosition.msecs;
    if (LIKELY(prev != -1))
        if (curr - prev < 100)
            return m_lastPosition.val + (curr - prev);

    if (!isRunning())
        return 0;

    const POT_DbusCall<qlonglong> f = [] (QDBusInterface* iface) -> QDBusReply<qlonglong> {
        return iface->call("Position");
    };
    const qint64 position = dbusSend<qlonglong>(&m_dbusIfaceProps, f, 0)/1000;
    m_lastPosition.msecs = QDateTime::currentMSecsSinceEpoch();
    m_lastPosition.val = position;

    return position;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::width
+-----------------------------------------------------------------------------*/
qint64 OMX_OmxplayerController::width()
{
    QMutexLocker locker(&m_mutex);

    const POT_DbusCall<qint64> f = [](QDBusInterface* iface) -> QDBusReply<qint64> {
        return iface->call("ResWidth");
    };

    return 1;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::streamPositionAsync
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::streamPositionAsync()
{
#if 0
    dbusSendAsync("Position", [this](QDBusPendingCallWatcher* w) {
        const QDBusPendingReply<qint64> reply = *w;
        const qint64 position = reply.value()/1000;
        delete w;

        emit streamPositionComputed(position);
    });
#endif

    const qint64 position = streamPosition();
    emit streamLengthComputed(position);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setGeometry
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::sendGeometry()
{
    if (status() != QMediaPlayer::BufferedMedia)
        return;
    if (m_process->state() != QProcess::Running)
        return;

    // FIXME: Can this be avoided?
    connectIfNeeded();

    {
        QMutexLocker l(&m_mutex);
        if (!is_connected(m_dbusIfacePlayer)) {
            QTimer::singleShot(100, this, [this] { m_dispatcher->dispatch(); });
            return;
        }
    }

    const POT_DbusCall<QString> f = [this] (QDBusInterface* iface) -> QDBusReply<QString> {
        QVariant path = QVariant::fromValue(QDBusObjectPath("/not/used"));
        QRect r;

        {
            QMutexLocker l(&m_geometryMutex);
            r = m_rect;
        }

        // It seems it is possible to call even outside of the mutex region.
        return iface->call("VideoPos", path, geometry_string(r));
    };

    dbusSend<QString>(&m_dbusIfacePlayer, f, QString());
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::loadInternal
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::loadInternal()
{
    QSize resolution = computeResolution(m_url);
    if (resolution.isNull()) {
        log_warn("Failed to parse resolution");
        m_url = QUrl();
        emit loadFailed();
        return;
    }

    setResolution(resolution);

    emit loadSucceeded();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::playInternal
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::playInternal()
{
    stop();
    kill_omxplayer();

    log_verbose("Playing...");
    if (m_process->state() != QProcess::NotRunning) {
        log_warn("OMX already running");
        return;
    }

    QStringList args = QStringList()
            << "--win"
            << geometry_string(m_rect)
            << m_url.toLocalFile();

    qDebug() << "omxplayer cmd line:" << args;
    m_process->start(PLAYER_COMMAND, args);
    if (m_process->waitForStarted(5000))
        emit playSucceeded();
    else {
        // TODO: Handle failure.
        log_warn("Failed to start omx");
    }

    QTimer::singleShot(0, this, SLOT(connectIfNeeded()));
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::eosReceived
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::eosReceived()
{
    setFrameVisible(false);
    emit stopped();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::startReceived
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::startReceived()
{
    setFrameVisible(true);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setX
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setX(qreal x)
{
    QMutexLocker l(&m_geometryMutex);

    m_rect.translate(qCeil(x - m_rect.x() - 1), 0);
    m_dispatcher->dispatch();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setY
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setY(qreal y)
{
    QMutexLocker l(&m_geometryMutex);

    m_rect.translate(0, qCeil(y - m_rect.y() - 1));
    m_dispatcher->dispatch();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setWidth
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setWidth(qreal w)
{
    QMutexLocker l(&m_geometryMutex);

    m_rect.setWidth(qCeil(w + 2));
    m_dispatcher->dispatch();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setHeight
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setHeight(qreal h)
{
    QMutexLocker l(&m_geometryMutex);

    m_rect.setHeight(qCeil(h + 2));
    m_dispatcher->dispatch();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setResolution
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setResolution(QSize resolution)
{
    if (m_resolution == resolution)
        return;
    m_resolution = resolution;
    emit resolutionChanged(m_resolution);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setStatus
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setStatus(QMediaPlayer::MediaStatus status)
{
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged(m_status);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setFrameVisible
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setFrameVisible(bool visible)
{
    if (m_frameVisible == visible)
        return;
    m_frameVisible = visible;
    emit frameVisibleChanged(visible);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::connectIfNeeded
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::connectIfNeeded()
{
    if (is_connected(m_dbusIfacePlayer) && is_connected(m_dbusIfaceProps))
        return;

    // Better to cache the connection.
    log_debug("Connecting to dbus interface....");
    delete m_dbusIfaceProps;
    delete m_dbusIfacePlayer;

#define DBUS_DEST \
    "org.mpris.MediaPlayer2.omxplayer"
#define DBUS_PATH \
    "/org/mpris/MediaPlayer2"
#define DBUS_FD_SERVICE \
    "org.freedesktop.DBus"
#define DBUS_MP_SERVICE \
    "org.mpris.MediaPlayer2"

    QDBusConnection bus = QDBusConnection::sessionBus();
    m_dbusIfaceProps = new QDBusInterface(
                DBUS_DEST,
                DBUS_PATH,
                DBUS_FD_SERVICE ".Properties",
                bus);
    m_dbusIfacePlayer = new QDBusInterface(
                DBUS_DEST,
                DBUS_PATH,
                DBUS_MP_SERVICE ".Player",
                bus);

    bus.connect("", "/redv/omx", "redv.omx.eos", "eos", this, SLOT(eosReceived()));
    bus.connect("", "/redv/omx", "redv.omx.started", "started", this, SLOT(startReceived()));
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::isRunning
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::isRunning()
{
    QMutexLocker locker(&m_mutex);
    return (m_process->state() != QProcess::NotRunning);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::dbusSend
+-----------------------------------------------------------------------------*/
template<typename T> T OMX_OmxplayerController::dbusSend(
        QDBusInterface** iface, POT_DbusCall<T> command, T failure)
{
    QMutexLocker locker(&m_mutex);

    //#define DBUS_PERF
#ifdef DBUS_PERF
    QElapsedTimer timer;
    timer.start();
#endif // DBUS_PERF

    connectIfNeeded();

#ifdef DBUS_PERF
    log_debug("Timer 1: %lld.", timer.restart());
#endif // DBUS_PERF

    // Check connection.
    if (!is_connected(*iface))
        return failure;

    const QDBusReply<T> r = command(*iface);
    if (!r.isValid()) {
        log_warn("Failure: %s", qPrintable(r.error().message()));
        return failure;
    }

#ifdef DBUS_PERF
    log_debug("Timer 2: %lld.", timer.restart());
#endif // DBUS_PERF

    return r.value();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::dbusSendAsync
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::dbusSendAsync(
        const QString& command, std::function<void(QDBusPendingCallWatcher*)> lambda)
{
    QMutexLocker locker(&m_mutex);

    connectIfNeeded();

    const QDBusPendingCall async =
            m_dbusIfaceProps->asyncCall(command);
    const QDBusPendingCallWatcher* watcher =
            new QDBusPendingCallWatcher(async);
    connect(watcher, &QDBusPendingCallWatcher::finished, lambda);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::computeResolution
+-----------------------------------------------------------------------------*/
QSize OMX_OmxplayerController::computeResolution(QUrl url)
{
    // Look into the xattr first.
    QByteArray pathData = url.toLocalFile().toUtf8();
    QSize size;
    if (getxattr(pathData.data(), "user.width", reinterpret_cast<void*>(&size.rwidth()), sizeof(int)) != -1) {
        if (getxattr(pathData.data(), "user.height", reinterpret_cast<void*>(&size.rheight()), sizeof(int)) != -1) {
            log_info("Found size in xattr: %dx%d", size.width(), size.height());
            return size;
        }
    }

    log_info("Computing size from %s...", qPrintable(url.toString()));
    QStringList args = QStringList()
            << "--info"
            << url.toString();

    QProcess proc;
    proc.start(PLAYER_COMMAND, args);
    proc.waitForFinished(5000);

    if (proc.exitCode() != 1) {
        log_warn("Failed to determine video res: %s",
                 qPrintable(QString::fromUtf8(proc.readAllStandardError())));
        return QSize();
    }

    QString ffmpegOutput = QString::fromUtf8(proc.readAllStandardError());
    QRegularExpression regex("Stream #.*Video:.*\\s([\\d]+)x([\\d]+)");
    QRegularExpressionMatch match = regex.match(ffmpegOutput);
    if (!match.hasMatch()) {
        log_warn("Failed to parse ffmpeg output: %s", qPrintable(ffmpegOutput));
        return QSize();
    }

    QString widthString = match.captured(1);
    QString heightString = match.captured(2);

    int width = widthString.toInt();
    int height = heightString.toInt();

    if (setxattr(pathData.data(), "user.width", reinterpret_cast<void*>(&width), sizeof(int), 0))
        qWarning() << "Failed to set xattr";
    if (setxattr(pathData.data(), "user.height", reinterpret_cast<void*>(&height), sizeof(int), 0))
        qWarning() << "Failed to set xattr";

    return QSize(width, height);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::updateDBusAddress
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::updateDBusAddress()
{
    QMutexLocker locker(&m_mutex);

    // Reset.
    m_dbusAddress = QString();
    qputenv("DBUS_SESSION_BUS_ADDRESS", m_dbusAddress.toLocal8Bit());

    QFile dbusfile(m_dbusFilePath);
    if (!dbusfile.exists())
        return;

    if (!dbusfile.open(QIODevice::ReadOnly)) {
        log_warn("Failed to open %s for reading.", qPrintable(m_dbusFilePath));
        return;
    }

    // Read.
    QTextStream stream(&dbusfile);
    QString dbusaddress = stream.readAll();
    if (NO_CONTENT(dbusaddress)) {
        log_warn("DBus file empty.");
        return;
    }

    m_dbusAddress = dbusaddress.trimmed();
    log_debug("Update dbus address to: %s.", qPrintable(m_dbusAddress));

    qputenv("DBUS_SESSION_BUS_ADDRESS", m_dbusAddress.toLocal8Bit());
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::updateDBusFilePath
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::updateDBusFilePath()
{
    QMutexLocker locker(&m_mutex);

    // Reset content.
    m_dbusFilePath = QString();

    // Read the file.
    QString username = qgetenv("USER");
    if (NO_CONTENT(username))
        username = qgetenv("USERNAME");
    if (NO_CONTENT(username)) {
        log_warn("Failed to get current user name.");
        return;
    }

#define OMXPLAYER_DBUS_F \
    "/tmp/omxplayerdbus."

    m_dbusFilePath = QString(OMXPLAYER_DBUS_F "%1").arg(username);
    log_debug("DBus file path: %s.", qPrintable(m_dbusFilePath));

    // Touch file otherwise QFileSystemWatcher won't work.
    QFile f(m_dbusFilePath);
    if (!f.exists())
        if (!f.open(QIODevice::WriteOnly))
            log_warn("Failed to create dbus file.");
    f.close();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::killProcess
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::killProcess()
{
    QProcess::execute("pkill", QStringList()
                      << "-P"
                      << QString::number(m_process->pid()));
    m_process->terminate();
    m_process->waitForFinished();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::prepareVideoLayer
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::prepareVideoLayer()
{
    std::call_once(g_prepare, [] {
        prepare_terminal();
    });
}

/*------------------------------------------------------------------------------
|    OMX_Process::~OMX_Process
+-----------------------------------------------------------------------------*/
OMX_Process::~OMX_Process()
{
    log_warn("QProcess freed");
}
