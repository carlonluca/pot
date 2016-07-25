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
#include <QWebView>
#include <QQuickView>
#include <QQuickItem>
#include <QGraphicsView>
#include <QGraphicsWebView>
#include <QGraphicsScene>
#include <QOpenGLWidget>
#include <QSurfaceFormat>

/*------------------------------------------------------------------------------
|    main
+-----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QApplication a(argc, argv);

	QStringList args = a.arguments();
	const bool opengl = !args.contains("--no-opengl");
	const bool wk2 = args.contains("--wk2");
	args.removeAll("--no-opengl");
	args.removeAll("--wk2");
	args.removeAll("--wk1"); // Default.

	const QString surl = args.at(1);

	if (!wk2) {
		QGraphicsWebView* webItem = new QGraphicsWebView;
		QOpenGLWidget* glViewport = new QOpenGLWidget;

		QGraphicsView* view = new QGraphicsView;

		// Set EGL to 24bit color depth.
		QSurfaceFormat curSurface = glViewport->format();
		curSurface.setRedBufferSize(8);
		curSurface.setGreenBufferSize(8);
		curSurface.setBlueBufferSize(8);
		curSurface.setAlphaBufferSize(0);
		glViewport->setFormat(curSurface);

		view->setRenderHints(QPainter::Antialiasing);
		view->setScene(new QGraphicsScene);
		if (opengl)
			view->setViewport(glViewport);
		else
			QObject::connect(qApp, SIGNAL(aboutToQuit()),
								  glViewport, SLOT(deleteLater()));
		view->showFullScreen();

		view->scene()->setBackgroundBrush(QBrush(Qt::red));
		view->scene()->setSceneRect(QRectF(0, 0, 1910, 1070));
		view->scene()->addItem(webItem);

		webItem->setUrl(QUrl(surl));
		webItem->setMinimumSize(1910, 1070);
	}
	else {
		QQuickView* view = new QQuickView;
		view->setSource(QUrl("qrc:/main_wk2.qml"));
		view->show();

		QObject* o = view->rootObject();
		o->setProperty("url", args.at(1));
	}

	return a.exec();
}
