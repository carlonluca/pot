/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.13.2013
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
*/

/*----------------------------------------------------------------------
|    includes
+---------------------------------------------------------------------*/
#include <QGuiApplication>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlContext>

#include "poc_utils.h"
#include "poc_qmlutils.h"

/*----------------------------------------------------------------------
|    main
+---------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    // Utility.
    POC_QMLUtils qmlUtils;

    QQuickView view;
    view.engine()->rootContext()->setContextProperty("utils", &qmlUtils);
    view.setSource(QUrl("qrc:///qml/main.qml"));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
#ifdef RASPBERRY
    view.showFullScreen();
#else
    view.resize(800, 400);
    view.show();
#endif // RASPBERRY
    qApp->connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    // If file path is provided from the command line, I start the player
    // immediately.
    QStringList args = app.arguments();
    if (args.size() > 1) {
       QFile f(args.at(1));
       if (!f.exists())
          qWarning("File provided does not exist.");
       else {
          QObject* rootObject  = dynamic_cast<QObject*>(view.rootObject());
          QObject* mediaPlayer = rootObject->findChild<QObject*>("mediaPlayer");
          mediaPlayer->setProperty("source", QUrl::fromLocalFile(args.at(1)));
          QMetaObject::invokeMethod(mediaPlayer, "play");
       }
    }

    return app.exec();
}
