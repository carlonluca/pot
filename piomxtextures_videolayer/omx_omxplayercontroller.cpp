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

#include "../piomxtextures_src/omx_globals.h"
#include "../3rdparty/lqtutils/lqtutils_string.h"
#include "omx_omxplayercontroller.h"
#include "omx_piomxtexturesplugin.h"
#include "omx_logging_cat.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#define NO_CONTENT(u) \
    (u.isEmpty() || u.isNull())

#define PLAYER_COMMAND \
    "omxplayer.bin"

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
|    orientation_cmd_value
+-----------------------------------------------------------------------------*/
inline QString orientation_cmd_value(OMX_OmxplayerController::Orientation orientation)
{
	switch (orientation) {
	case OMX_OmxplayerController::ROT_90:
		return QStringLiteral("90");
	case OMX_OmxplayerController::ROT_180:
		return QStringLiteral("180");
	case OMX_OmxplayerController::ROT_270:
		return QStringLiteral("270");
	case OMX_OmxplayerController::ROT_0:
	default:
		return QStringLiteral("0");
	}
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
#ifdef CLEAR_TERM
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
#endif

#ifdef USE_SIG_HANDLERS
typedef void (*sighandler)(int);

static QMap<int, sighandler> g_handlers;

/*------------------------------------------------------------------------------
|    sig_handler
+-----------------------------------------------------------------------------*/
static std::once_flag g_handler;

static void sig_handler(int s)
{
    Q_UNUSED(s)

    std::call_once(g_handler, [s] {
#if 0
        log_info("Received signal %d - %s", s, strsignal(s));
        log_stacktrace(LOG_TAG, LC_LogLevel::LC_LOG_INFO, 1000);
        log_info("Cleaning up...");
#endif
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

#ifdef CLEAR_TERM
    setterm(T_C_BLACK);
#endif

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
|    OMX_Process::schedule
+-----------------------------------------------------------------------------*/
void OMX_CommandProcessor::schedule(const OMX_CommandProcessor::Command& cmd)
{
    qCDebug(vl) << Q_FUNC_INFO << cmd.type;
    QMutexLocker locker(&m_mutex);
    m_pending.enqueue(cmd);
    m_cond.wakeOne();
}

/*------------------------------------------------------------------------------
|    OMX_Process::run
+-----------------------------------------------------------------------------*/
void OMX_CommandProcessor::run()
{
    m_ready.acquire();

    while (!isInterruptionRequested()) {
        m_mutex.lock();
        if (m_pending.isEmpty())
            m_cond.wait(&m_mutex);
        Command cmd = m_pending.dequeue();
        m_mutex.unlock();

        if (cmd.type == CMD_CLEAN_THREAD)
            return;
        if (isInterruptionRequested())
            return;

        qCDebug(vl) << "Processing command" << cmd.type;
        m_controller->processCommand(cmd);
    }
}

/*------------------------------------------------------------------------------
|    OMX_Process::ready
+-----------------------------------------------------------------------------*/
void OMX_CommandProcessor::ready()
{
    m_ready.release();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::OMX_OmxplayerController
+-----------------------------------------------------------------------------*/
OMX_OmxplayerController::OMX_OmxplayerController(QObject* parent) :
    QObject(parent)
  , m_cmdProc(new OMX_CommandProcessor(this))
  , m_dbusIfaceProps(nullptr)
  , m_dbusIfacePlayer(nullptr)
  , m_thread(new QThread)
  , m_rect(QPoint(0, 0), QGuiApplication::primaryScreen()->size())
  , m_fillMode(Qt::IgnoreAspectRatio)
  , m_lastDuration({-1, -1})
  , m_lastPosition({-1, -1})
  , m_status(QMediaPlayer::NoMedia)
  , m_frameVisible(false)
  , m_muted(false)
  , m_vol(0)
  , m_orientation(ROT_0)
  , m_stateNoMedia(new QState)
  , m_stateLoading(new QState)
  , m_stateLoaded(new QState)
  , m_statePlaying(new QState)
  , m_stateStopping(new QState)
  , m_statePaused(new QState)
  , m_stateEom(new QState)
{
    log_debug_func;

    static int index = 0;
    static QMutex generateIndex;
    {
        generateIndex.lock();
        m_dbusService = QString("org.mpris.MediaPlayer2.instance%0.omxplayer").arg(index++);
        qCDebug(vl) << "Service: " << m_dbusService;
        generateIndex.unlock();
    }

    prepareVideoLayer();

    connect(this, SIGNAL(destroyed()),
            m_thread, SLOT(quit()));
    connect(m_thread, SIGNAL(finished()),
            m_thread, SLOT(deleteLater()));

    m_dbusConnMonitor = new QTimer;
    m_dbusConnMonitor->setInterval(1000);
    m_dbusConnMonitor->setSingleShot(false);
    connect(m_dbusConnMonitor, &QTimer::timeout,
            this, &OMX_OmxplayerController::connectIfNeeded);

    m_process = new QProcess;
	connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this] {
		setFrameVisible(false);
        log_verbose("omx process closed");
    });
    connect(m_process, &QProcess::stateChanged, this, [this] (QProcess::ProcessState state) {
        if (state == QProcess::Running)
            m_dbusConnMonitor->start();
        else
            m_dbusConnMonitor->stop();
    });

    m_cmdProc->start();

    // Start the dispatcher.
    m_dispatcher = new OMX_GeometryDispatcher(this);
    m_dispatcher->start();

    connect(m_dispatcher, SIGNAL(sendGeometry()),
            this, SLOT(sendGeometry()));

    // No media.
    connect(m_stateNoMedia, &QState::entered, [this] {
        log_verbose("State entered: STATE_NO_MEDIA");
        setMediaStatus(QMediaPlayer::NoMedia);
        setPlaybackState(QMediaPlayer::StoppedState);
        m_cmdProc->ready();
    });
    m_stateNoMedia->addTransition(this, SIGNAL(loadRequested()), m_stateLoading);

    // Loading.
    connect(m_stateLoading, &QState::entered, [this] {
        log_verbose("State entered: STATE_LOADING");
        setMediaStatus(QMediaPlayer::LoadingMedia);
        setPlaybackState(QMediaPlayer::StoppedState);
        killProcess();
        loadInternal();
    });
    m_stateLoading->addTransition(this, SIGNAL(loadSucceeded()), m_stateLoaded);
    m_stateLoading->addTransition(this, SIGNAL(loadFailed()), m_stateNoMedia);
    // TODO: Handle failure here.

    // Loaded.
    connect(m_stateLoaded, &QState::entered, [this] {
        log_verbose("State entered: STATE_LOADED");
        killProcess();
        setMediaStatus(QMediaPlayer::LoadedMedia);
        setPlaybackState(QMediaPlayer::StoppedState);
    });
    m_stateLoaded->addTransition(this, SIGNAL(playRequested()), m_statePlaying);
    m_stateLoaded->addTransition(this, SIGNAL(loadRequested()), m_stateLoading);

    // Playing.
    connect(m_statePlaying, &QState::entered, [this] {
        log_verbose("State entered: PLAYING");
        int position = m_lastPlayCommand.data.isNull() ? 0 : m_lastPlayCommand.data.toInt();
        playInternal(position);
        setMediaStatus(QMediaPlayer::BufferedMedia);
        setPlaybackState(QMediaPlayer::PlayingState);
    });
    m_statePlaying->addTransition(this, SIGNAL(finished()), m_stateEom);
    m_statePlaying->addTransition(this, SIGNAL(interrupted()), m_stateLoaded);
    m_statePlaying->addTransition(this, SIGNAL(pauseRequested()), m_statePaused);
    m_statePlaying->addTransition(this, SIGNAL(loadRequested()), m_stateStopping);
    m_statePlaying->addTransition(this, SIGNAL(stopRequested()), m_stateStopping);

    connect(m_stateStopping, &QState::entered, [this] {
        log_verbose("State entered: STOPPING");
        stopInternal();
        emit interrupted();
    });
    m_stateStopping->addTransition(this, SIGNAL(interrupted()), m_stateLoaded);

    // Paused.
    connect(m_statePaused, &QState::entered, [this] {
        log_verbose("State entered: PAUSED");
        pauseInternal();
        setMediaStatus(QMediaPlayer::BufferedMedia);
        setPlaybackState(QMediaPlayer::PausedState);
    });
    m_statePaused->addTransition(this, SIGNAL(playRequested()), m_statePlaying);
    m_statePaused->addTransition(this, SIGNAL(stopRequested()), m_stateLoaded);

    // EOM.
    connect(m_stateEom, &QState::entered, [this] {
        log_verbose("State entered: EOM");
        setMediaStatus(QMediaPlayer::EndOfMedia);
        setPlaybackState(QMediaPlayer::StoppedState);
    });
    m_stateEom->addTransition(this, SIGNAL(loadRequested()), m_stateLoading);
    m_stateEom->addTransition(this, SIGNAL(playRequested()), m_statePlaying);
    m_stateEom->addTransition(this, SIGNAL(stopRequested()), m_stateLoaded);

    m_machine = new QStateMachine(this);
    m_machine->addState(m_stateNoMedia);
    m_machine->addState(m_stateLoading);
    m_machine->addState(m_stateLoaded);
    m_machine->addState(m_statePlaying);
    m_machine->addState(m_statePaused);
    m_machine->addState(m_stateEom);
    m_machine->addState(m_stateStopping);
    m_machine->setInitialState(m_stateNoMedia);
    m_machine->start();

#ifdef USE_SIG_HANDLERS
    install_handlers();
#endif // USE_SIG_HANDLERS

    m_dbusConnMonitor->moveToThread(m_thread);
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
        disconnect(m_process, nullptr, nullptr, nullptr);
        killProcess();
        m_process->waitForFinished(5000);
        delete m_process;
    }

    m_machine->stop();

    m_dispatcher->requestInterruption();
    m_dispatcher->dispatch();
    m_dispatcher->wait(5000);
    delete m_dispatcher;

    m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_CLEAN_THREAD));
    m_cmdProc->requestInterruption();
    m_cmdProc->wait(5000);
    delete m_cmdProc;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setFilename
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::setFilenameInternal(QUrl url)
{
    QMutexLocker locker(&m_mutex);
    if (url.isEmpty() || !url.isValid()) {
        m_url = QUrl();
        emit loadFailed();
        return false;
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

    emit loadRequested();

    return true;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setLayer
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::setLayer(int layer)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(layer)

    // TODO: Implement.

    return false;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::play
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::play(int position)
{
    log_debug_func;
    OMX_CommandProcessor::Command cmd(OMX_CommandProcessor::CMD_PLAY);
    cmd.data = position;
    m_cmdProc->schedule(cmd);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::stop
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::stop()
{
    m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_STOP));
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::pause
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::pause()
{
    m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_PAUSE));
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setPosition
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::setPosition(qint64 microsecs)
{
    QMutexLocker locker(&m_mutex);
    if (m_process->state() != QProcess::Running)
        return false;
    const POT_DbusCall<qint64> f = [microsecs] (QDBusInterface* iface) -> QDBusReply<qint64> {
        return iface->call(QStringLiteral("SetPosition"),
                           QVariant::fromValue(QDBusObjectPath(QStringLiteral("/dev/null"))),
                           QVariant::fromValue(microsecs));
    };
    return dbusSend<qint64>(m_dbusIfacePlayer.get(), f, 0) == 0;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setSource
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setSource(const QUrl& url)
{
#if 0
    if (playbackState() == QMediaPlayer::PlayingState) {
        m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_STOP));
        m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_SET_SOURCE, url));
        m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_PLAY));
    }
    else
        m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_SET_SOURCE, url));
#endif

    if (playbackState() == QMediaPlayer::PlayingState)
        m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_STOP));
    m_cmdProc->schedule(OMX_CommandProcessor::Command(OMX_CommandProcessor::CMD_SET_SOURCE, url));
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
        return iface->call(QStringLiteral("Duration"));
    };
    const qint64 duration = dbusSend<qint64>(m_dbusIfaceProps.get(), f, 0)/1000;
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

    const POT_DbusCall<qint64> f = [] (QDBusInterface* iface) -> QDBusReply<qint64> {
        return iface->call("Position");
    };
    const qint64 position = dbusSend<qint64>(m_dbusIfaceProps.get(), f, 0)/1000L;
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
    if (mediaStatus() != QMediaPlayer::BufferedMedia)
        return;
    if (m_process->state() != QProcess::Running)
        return;

    // FIXME: Can this be avoided?
    //connectIfNeeded();

    {
        QMutexLocker l(&m_mutex);
        if (!is_connected(m_dbusIfacePlayer.get())) {
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

    dbusSend<QString>(m_dbusIfacePlayer.get(), f, QString());
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
void OMX_OmxplayerController::playInternal(int position)
{
    if (playbackState() == QMediaPlayer::PausedState) {
        const POT_DbusCallVoid f = [] (QDBusInterface* iface) -> QDBusReply<void> {
            return iface->call("Play");
        };
        dbusSend(m_dbusIfacePlayer.get(), f);
        return;
    }

    log_verbose("Playing...");
    if (m_process->state() != QProcess::NotRunning) {
        log_warn("OMX already running");
        return;
    }

    m_vol = (m_muted ? -6000 : 0);

    QStringList customArgs = readOmxplayerArguments();
    QString layer = readOmxLayer();
    QStringList args = QStringList()
            << "--layer" << layer
            << "--dbus_name" << m_dbusService
            << "--vol" << QString::number(m_vol)
            << "--orientation" << orientation_cmd_value(m_orientation)
            << "--win"
            << geometry_string(m_rect)
            << customArgs
            << QSL("-l") << QString::number(qRound(position/1000.0));
    if (m_loop)
        args << QSL("--loop");
    args << m_url.toLocalFile();

    qCDebug(vl) << "omxplayer cmd line:" << args;
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
|    OMX_OmxplayerController::pauseInternal
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::pauseInternal()
{
    QMutexLocker locker(&m_mutex);
    if (m_process->state() != QProcess::Running)
        return;

    const POT_DbusCallVoid f = [] (QDBusInterface* iface) -> QDBusReply<void> {
        return iface->call("Pause");
    };
    dbusSend(m_dbusIfacePlayer.get(), f);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::stopInternal
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::stopInternal()
{
    QMutexLocker locker(&m_mutex);
    switch (m_process->state()) {
    case QProcess::Running:
        killProcess();
        break;
    case QProcess::Starting:
        m_process->waitForStarted(5000);
        killProcess();
        emit interrupted();
        break;
    case QProcess::NotRunning:
        qCDebug(vl, "Cannot stop, process already dead");
        break;
    }
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::eosReceived
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::eosDbusReceived()
{
    qCDebug(vl) << Q_FUNC_INFO;

    //if (failure)
    //    emit interrupted();
    //else
        emit finished();

    setFrameVisible(false);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::startReceived
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::startDbusReceived()
{
    qCDebug(vl) << Q_FUNC_INFO;

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
+----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setResolution(QSize resolution)
{
    if (m_resolution == resolution)
        return;
    m_resolution = resolution;
    emit resolutionChanged(m_resolution);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setMuted
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setMuted(bool muted)
{
    bool changed = false;

    if (m_muted != muted) {
        qCDebug(vl) << Q_FUNC_INFO << muted;
        m_muted = muted;
        dbusSend(m_dbusIfacePlayer.get(), [muted] (QDBusInterface* iface) -> QDBusReply<void> {
            return iface->call(muted ? "Mute" : "Unmute");
        });
        changed = true;
    }

    int expectedVol = (muted ? -6000 : 0);
    if (m_vol != expectedVol) {
        qCDebug(vl) << Q_FUNC_INFO << expectedVol;
        m_vol = expectedVol;
        dbusSend(m_dbusIfaceProps.get(), [expectedVol] (QDBusInterface* iface) -> QDBusReply<void> {
            return iface->call("Volume", qPow(10, expectedVol/2000.0));
        });
        changed = true;
    }

    if (changed)
        emit mutedChanged(m_muted);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setPlaybackState
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setPlaybackState(OMX_MediaPlayerState state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit playbackStateChanged(m_state);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setMediaStatus
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setMediaStatus(QMediaPlayer::MediaStatus status)
{
    if (m_status == status)
        return;
    m_status = status;
    emit mediaStatusChanged(m_status);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::setFrameVisible
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::setFrameVisible(bool visible)
{
    if (m_frameVisible == visible)
        return;

    qCDebug(vl) << Q_FUNC_INFO << visible;
    m_frameVisible = visible;
    emit frameVisibleChanged(visible);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::connectIfNeeded
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::connectIfNeeded()
{
    QMutexLocker locker(&m_mutex);

    if (is_connected(m_dbusIfacePlayer.get()) && is_connected(m_dbusIfaceProps.get())) {
        log_verbose("dbus connection already established");
        return;
    }

    // Better to cache the connection.
    log_debug("Connecting to dbus interface....");

//#define DBUS_DEST "org.mpris.MediaPlayer2.omxplayer"
#define DBUS_PATH \
    "/org/mpris/MediaPlayer2"
#define DBUS_FD_SERVICE \
    "org.freedesktop.DBus"
#define DBUS_MP_SERVICE \
    "org.mpris.MediaPlayer2"

    QDBusConnection bus = QDBusConnection::connectToBus(OMX_PiOmxTexturesPlugin::dbusAddress,
                                                        QStringLiteral("omx_connection"));
    if (!bus.isConnected()) {
        qCWarning(vl) << "Cannot connect to session bus";
        return;
    }

    m_dbusIfaceProps.reset(new QDBusInterface(
                m_dbusService,
                DBUS_PATH,
                DBUS_FD_SERVICE ".Properties",
                bus));
    m_dbusIfacePlayer.reset(new QDBusInterface(
                m_dbusService,
                DBUS_PATH,
                DBUS_MP_SERVICE ".Player",
                bus));

    qCInfo(vl) << "Connecting signals";
    bus.connect(m_dbusService, "/redv/omx", "redv.omx.eos", "eos", this, SLOT(eosDbusReceived()));
    bus.connect(m_dbusService, "/redv/omx", "redv.omx.started", "started", this, SLOT(startDbusReceived()));

    QTimer::singleShot(0, this, [this] {
        const POT_DbusCall<bool> f = [] (QDBusInterface* iface) -> QDBusReply<bool> {
            return iface->call(QStringLiteral("Rendering"));
        };
        setFrameVisible(dbusSend<bool>(m_dbusIfaceProps.get(), f, 0));
    });
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
        QDBusInterface* iface, POT_DbusCall<T> command, T failure)
{
    QMutexLocker locker(&m_mutex);

    //#define DBUS_PERF
#ifdef DBUS_PERF
    QElapsedTimer timer;
    timer.start();
#endif // DBUS_PERF

    //connectIfNeeded();

#ifdef DBUS_PERF
    log_debug("Timer 1: %lld.", timer.restart());
#endif // DBUS_PERF

    // Check connection.
    if (!is_connected(iface)) {
        log_warn("Not yet connected");
        return failure;
    }

    const QDBusReply<T> r = command(iface);
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
|    OMX_OmxplayerController::dbusSend
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::dbusSend(
    QDBusInterface* iface, POT_DbusCallVoid command)
{
    QMutexLocker locker(&m_mutex);
    //connectIfNeeded();
    if (!is_connected(iface))
        return false;
    const QDBusReply<void> r = command(iface);
    if (!r.isValid()) {
        log_warn("Failure: %s", qPrintable(r.error().message()));
        return false;
    }

    return true;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::dbusSendAsync
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::dbusSendAsync(
        const QString& command, std::function<void(QDBusPendingCallWatcher*)> lambda)
{
    QMutexLocker locker(&m_mutex);

    //connectIfNeeded();

    const QDBusPendingCall async =
            m_dbusIfaceProps->asyncCall(command);
    const QDBusPendingCallWatcher* watcher =
            new QDBusPendingCallWatcher(async);
    connect(watcher, &QDBusPendingCallWatcher::finished, lambda);
    connect(watcher, &QDBusPendingCallWatcher::finished, &QObject::deleteLater);
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
            log_debug("Found size in xattr: %dx%d", size.width(), size.height());
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
        qCWarning(vl) << "Failed to set xattr";
    if (setxattr(pathData.data(), "user.height", reinterpret_cast<void*>(&height), sizeof(int), 0))
        qCWarning(vl) << "Failed to set xattr";

    return QSize(width, height);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::processCommand
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::processCommand(const OMX_CommandProcessor::Command& cmd)
{
    switch (cmd.type) {
    case OMX_CommandProcessor::CMD_PLAY: {
        m_lastPlayCommand = cmd;
        emit playRequested();
        QSet<QAbstractState*> states { m_statePaused, m_stateLoaded };
        if (isInStates(states))
            waitForStates(QSet<QAbstractState*> { m_statePlaying });
        break;
    }
    case OMX_CommandProcessor::CMD_STOP: {
        emit stopRequested();
        QSet<QAbstractState*> states { m_statePlaying, m_statePaused, m_stateEom };
        if (isInStates(states))
            waitForStates(QSet<QAbstractState*> { m_stateLoaded });
        break;
    }
    case OMX_CommandProcessor::CMD_PAUSE: {
        emit pauseRequested();
        QSet<QAbstractState*> states { m_statePlaying };
        if (isInStates(states))
            waitForStates(QSet<QAbstractState*> { m_statePaused });
        break;
    }
    case OMX_CommandProcessor::CMD_SET_SOURCE: {
        qCDebug(vl) << cmd.data.toUrl();
        if (!setFilenameInternal(cmd.data.toUrl()))
            break;
        QSet<QAbstractState*> states { m_stateNoMedia, m_stateLoading, m_stateLoaded, m_statePlaying, m_stateEom };
        if (isInStates(states))
            waitForStates(QSet<QAbstractState*> { m_stateLoaded });
        break;
    }
    case OMX_CommandProcessor::CMD_CLEAN_THREAD:
        break;
    case OMX_CommandProcessor::CMD_NONE:
        break;
    }

    qCDebug(vl) << "Command completed:" << cmd.type;
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::isInStates
+-----------------------------------------------------------------------------*/
bool OMX_OmxplayerController::isInStates(const QSet<QAbstractState*> states)
{
    return !m_machine->configuration().intersect(states).isEmpty();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::waitForStates
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::waitForStates(const QSet<QAbstractState*> states)
{
    QEventLoop loop;
    foreach (QAbstractState* state, states) {
        connect(state, &QAbstractState::entered,
                &loop, &QEventLoop::quit);
    }
    loop.exec();
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::readOmxplayerArguments
+-----------------------------------------------------------------------------*/
QStringList OMX_OmxplayerController::readOmxplayerArguments()
{
    QByteArray data = qgetenv("POT_OMXPLAYER_ARGS");
    if (data.isEmpty())
        return QStringList();

    QString argString = QString(data);
#if QT_VERSION_MAJOR > 5
    return argString.split(" ", Qt::SkipEmptyParts);
#else
	return argString.split(" ", QString::SkipEmptyParts);
#endif
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::readOmxLayer
+-----------------------------------------------------------------------------*/
QString OMX_OmxplayerController::readOmxLayer()
{
	const QString defaultLayer = QStringLiteral("-128");

	QByteArray data = qgetenv("POT_LAYER");
	if (data.isEmpty())
		return defaultLayer;
	return QString(data);
}

/*------------------------------------------------------------------------------
|    OMX_OmxplayerController::killProcess
+-----------------------------------------------------------------------------*/
void OMX_OmxplayerController::killProcess()
{
    if (m_process->state() == QProcess::Running) {
        QProcess::execute("pkill", QStringList()
                        << "--signal" << "SIGINT"
                        << "-P"
                        << QString::number(m_process->processId()));
        m_process->kill();
        m_process->waitForFinished();
    }
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
