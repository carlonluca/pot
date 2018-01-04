/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QApplication>
#include <QWebView>
#include <QWebFrame>
#include <QGraphicsWebView>
#include <QGraphicsView>
#include <QOpenGLWidget>
#include <QSurfaceFormat>

#include <lc_logging.h>

#include "poc_bridge.h"

using namespace lightlogger;

/*------------------------------------------------------------------------------
|    main
+-----------------------------------------------------------------------------*/
int main(int argc, char** argv) {
	qputenv("QT_QPA_EGLFS_FORCE888", "1");

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QApplication a(argc, argv);

	const QStringList args = a.arguments();
	if (args.size() < 2) {
		log_err("Video ID is missing.");
		return 1;
	}

	const QString videoId = args.at(1);
	log_info("Starting playback of video ID %s...", qPrintable(videoId));

	POC_Bridge bridge(videoId);

	QGraphicsWebView* webItem = new QGraphicsWebView;
	webItem->page()->mainFrame()->addToJavaScriptWindowObject("bridge", &bridge);

	QOpenGLWidget* glViewport = new QOpenGLWidget;

	QGraphicsView* view = new QGraphicsView;
	view->setFrameShape(QFrame::NoFrame);

	view->setRenderHints(QPainter::Antialiasing);
	view->setScene(new QGraphicsScene);
	view->setViewport(glViewport);
	view->showFullScreen();

	view->scene()->setBackgroundBrush(QBrush(Qt::red));
	view->scene()->setSceneRect(QRectF(0, 0, 1920, 1080));
	view->scene()->addItem(webItem);

	webItem->setUrl(QUrl("qrc:/player.html"));
	webItem->setMinimumSize(1920, 1080);

	return a.exec();
}
