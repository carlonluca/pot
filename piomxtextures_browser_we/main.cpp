/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    11.29.2015
 *
 * Copyright (c) 2015 Luca Carlon. All rights reserved.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
 */

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QApplication>
#include <QQuickView>
#include <QQuickItem>
#include <QtWebEngineWidgets>

/*------------------------------------------------------------------------------
|    main
+-----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QApplication a(argc, argv);

	QStringList args = a.arguments();
	const bool opengl = !args.contains("--no-opengl");
	args.removeAll("--no-opengl");

	if (opengl) {
		qDebug("QML QtWebEngine...");

		QQuickView* view = new QQuickView;
		view->setSource(QUrl("qrc:/poc_main.qml"));
		view->showFullScreen();

		QObject* o = view->rootObject()->findChild<QObject*>("webEngineView");
		o->setProperty("url", args.at(1));
	}
	else {
		qDebug("Widget QtWebEngine...");

		QWebEngineView* view = new QWebEngineView;
		view->load(QUrl(args.at(1)));
		view->show();
	}

	return a.exec();
}
