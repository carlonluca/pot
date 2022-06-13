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

#ifndef OMX_PIOMXTEXTURESPLUGIN_H
#define OMX_PIOMXTEXTURESPLUGIN_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QQmlExtensionPlugin>
#include <QtQml>
#include <QObject>
#include <QProcess>
#include <QCoreApplication>
#include <QDebug>
#include <QTextStream>

#include "omx_videolayer.h"
#include "omx_video.h"
#include "omx_audio.h"
#include "omx_logging_cat.h"

/*------------------------------------------------------------------------------
|    OMX_PiOmxTexturesPlugin class
+-----------------------------------------------------------------------------*/
class OMX_PiOmxTexturesPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    OMX_PiOmxTexturesPlugin(QObject* parent = nullptr);

    void registerTypes(const char* uri) {
        registerLibraryTypes(uri);
    }

    static void registerLibraryTypes(const char* uri) {
#ifdef VERSION
        log_info("PiOmxTexturesVideoLayer version %s built %s, %s.",
                 VERSION, __DATE__, __TIME__);
#endif

        log_verbose("Registering OMX_VideoLayer QML type...");
        qmlRegisterType<OMX_VideoLayer>(uri, 0, 1, "POT_VideoLayer");
        qmlRegisterType<OMX_Video>(uri, 0, 1, "POT_Video");
        qmlRegisterType<OMX_Audio>(uri, 0, 1, "POT_Audio");
        qmlRegisterType<OMX_Video>(uri, 0, 1, "Video");
        qmlRegisterType<OMX_Audio>(uri, 0, 1, "Audio");
        qmlRegisterType<MediaPlayer>(uri, 0, 1, "MediaPlayer");

        // Spawn dbus-daemon.
        QProcess* dbusDaemon = new QProcess;
        connect(qApp, &QCoreApplication::aboutToQuit,
                dbusDaemon, &QProcess::kill);
        dbusDaemon->start(QStringLiteral("dbus-daemon"), QStringList()
                          << QStringLiteral("--session")
                          << QStringLiteral("--print-address"));
        dbusDaemon->waitForReadyRead();

        QString address = dbusDaemon->readAllStandardOutput();
        if (address.isNull()) {
            qCritical("Failed to execute dbus-daemon");
            qApp->exit(1);
        }

        dbusAddress = address.trimmed();

        log_debug("Update dbus address to: %s.", qPrintable(dbusAddress));
        qputenv("DBUS_SESSION_BUS_ADDRESS", dbusAddress.toLocal8Bit());

        QFile f("/tmp/omxplayerdbus.touch");
        if (!f.open(QIODevice::WriteOnly)) {
            qCritical("Failed to store dbus address");
            qApp->exit(1);
        }

        QTextStream s(&f);
        s << address;
        s.flush();
    }

    static QString dbusAddress;
};

#endif // OMX_PIOMXTEXTURESPLUGIN_H
