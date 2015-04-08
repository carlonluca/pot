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

#include "lc_logging.h"

#include "poc_utils.h"
#include "poc_qmlutils.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
enum POC_Mode {
	MODE_PLAYER,
	MODE_LOOP,
	MODE_ANIMATIONS
};

/*------------------------------------------------------------------------------
|    show_local_media
+-----------------------------------------------------------------------------*/
bool show_media(QQuickView* view, QStringList mediaList)
{
	for (int i = 0; i < mediaList.size(); i++)
		if (!QFile(mediaList[i]).exists())
			return log_warn("File %s does not exist.", qPrintable(mediaList[i]));

	QObject* rootObject  = dynamic_cast<QObject*>(view->rootObject());
	QMetaObject::invokeMethod(rootObject, "showLocalMedia", Q_ARG(QVariant, mediaList));

	return true;
}

/*------------------------------------------------------------------------------
|    show_local_media
+-----------------------------------------------------------------------------*/
bool show_media(QQuickView* view, QString fileUri)
{
	QObject* rootObject  = dynamic_cast<QObject*>(view->rootObject());
	QObject* mediaOutput = rootObject->findChild<QObject*>("mediaOutput");
   QMetaObject::invokeMethod(mediaOutput, "showUrlMedia", Q_ARG(QVariant, fileUri));

	return true;
}

/*----------------------------------------------------------------------
|    main
+---------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);

	// Utility.
	QStringList args = app.arguments();
	POC_Mode currentMode;
	if (args.contains("--animations"))
		currentMode = MODE_ANIMATIONS;
	else if (args.contains("--loop"))
		currentMode = MODE_LOOP;
	else
		currentMode = MODE_PLAYER;

	POC_QMLUtils qmlUtils;

	QQuickView view;

   // Set EGL to 24bit color depth.
   QSurfaceFormat curSurface = view.format();
   curSurface.setRedBufferSize(8);
   curSurface.setGreenBufferSize(8);
   curSurface.setBlueBufferSize(8);
   curSurface.setAlphaBufferSize(0);
   view.setFormat(curSurface);

	view.engine()->rootContext()->setContextProperty("utils", &qmlUtils);
	switch (currentMode) {
	case MODE_ANIMATIONS:
		view.setSource(QUrl("qrc:///qml/main_animations.qml"));
		break;
	case MODE_LOOP:
		view.setSource(QUrl("qrc:///qml/main_loop.qml"));
		break;
	default:
		view.setSource(QUrl("qrc:///qml/main.qml"));
		break;
	}

	qInstallMessageHandler(&log_handler);
	LC_QMLLogger::registerObject(view.rootContext());

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
	switch (currentMode) {
	case MODE_LOOP: {
		QStringList list;
		for (int i = 2; i < args.size(); i++)
			list << args.at(i);
		if (list.size() < 1)
			return log_warn("No items to play.");
      if (!show_media(&view, list))
			return 1;
		break;
	}
	default:
      if (args.size() > 1)
         if (!show_media(&view, args.at(1)))
				return 1;
		break;
	}

	return app.exec();
}
