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
#include "poc_uptime.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
enum POC_Mode {
	MODE_PLAYER,
	MODE_LOOP,
	MODE_ANIMATIONS,
	MODE_SEEK,
	MODE_MULTIPLE,
	MODE_MULTIPLEANIM,
	MODE_OVERLAYS
};

/*------------------------------------------------------------------------------
|    show_local_media
+-----------------------------------------------------------------------------*/
bool show_media(QQuickView* view, QStringList mediaList)
{
   QStringList uriList;
   for (int i = 0; i < mediaList.size(); i++) {
		if (!QFile(mediaList[i]).exists())
			return log_warn("File %s does not exist.", qPrintable(mediaList[i]));
      uriList.append(QUrl::fromUserInput(mediaList[i]).toString());
   }

	QObject* rootObject  = dynamic_cast<QObject*>(view->rootObject());
   QMetaObject::invokeMethod(rootObject, "showLocalMedia", Q_ARG(QVariant, uriList));

	return true;
}

/*------------------------------------------------------------------------------
|    show_local_media
+-----------------------------------------------------------------------------*/
bool show_media(QQuickView* view, QString mediaLocation)
{
   log_info("Showing media: %s.", qPrintable(mediaLocation));
   QUrl uri = QUrl::fromUserInput(mediaLocation);

	QObject* rootObject  = dynamic_cast<QObject*>(view->rootObject());
	QObject* mediaOutput = rootObject->findChild<QObject*>("mediaOutput");
   QMetaObject::invokeMethod(mediaOutput, "showUrlMedia", Q_ARG(QVariant, uri.toString()));

	return true;
}

/*----------------------------------------------------------------------
|    main
+---------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QGuiApplication app(argc, argv);

	// Utility.
	QStringList args = app.arguments();
	POC_Mode currentMode;
	if (args.contains("--animations"))
		currentMode = MODE_ANIMATIONS;
	else if (args.contains("--loop"))
		currentMode = MODE_LOOP;
	else if (args.contains("--seektest"))
		currentMode = MODE_SEEK;
	else if (args.contains("--multipletest"))
		currentMode = MODE_MULTIPLE;
	else if (args.contains("--multipleanimtest"))
		currentMode = MODE_MULTIPLEANIM;
	else if (args.contains("--overlaystest"))
		currentMode = MODE_OVERLAYS;
	else
		currentMode = MODE_PLAYER;

	POC_QMLUtils qmlUtils;
	POC_Uptime uptime;

	QQuickView view;

	// Set EGL to 24bit color depth.
	QSurfaceFormat curSurface = view.format();
	curSurface.setRedBufferSize(8);
	curSurface.setGreenBufferSize(8);
	curSurface.setBlueBufferSize(8);
	curSurface.setAlphaBufferSize(0);
	view.setFormat(curSurface);

	view.engine()->rootContext()->setContextProperty("utils", &qmlUtils);
	view.engine()->rootContext()->setContextProperty("uptime", &uptime);

	switch (currentMode) {
	case MODE_ANIMATIONS:
		view.setSource(QUrl(QStringLiteral("qrc:///qml/main_animations.qml")));
		break;
	case MODE_LOOP:
		view.setSource(QUrl(QStringLiteral("qrc:///qml/main_loop.qml")));
		break;
	case MODE_SEEK:
		view.setSource(QUrl(QStringLiteral("qrc:///qml/main_seektest.qml")));
		break;
	case MODE_MULTIPLE:
		view.setSource(QUrl(QStringLiteral("qrc:///qml/main_multiple.qml")));
		break;
	case MODE_OVERLAYS:
		view.setSource(QUrl(QStringLiteral("qrc:///qml/main_overlays.qml")));
		break;
	case MODE_MULTIPLEANIM:
		view.setSource(QUrl(QStringLiteral("qrc:///qml/main_multipleanim.qml")));
		break;
	default:
		view.setSource(QUrl(QStringLiteral("qrc:///qml/main.qml")));
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
	case MODE_MULTIPLEANIM:
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
	case MODE_PLAYER:
		if (args.size() > 1)
			if (!show_media(&view, args.at(1)))
				return 1;
		break;
	case MODE_MULTIPLE:
		break;
	default:
		if (args.size() > 2)
			if (!show_media(&view, args.at(2)))
				return 1;
		break;
	}

	return app.exec();
}
