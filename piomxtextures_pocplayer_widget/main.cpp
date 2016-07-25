#include <QApplication>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QOpenGLWidget>

#include <QImage>

int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QApplication a(argc, argv);

	const QString fileAbsPath = a.arguments().at(1);
	qDebug("Opening %s...", qPrintable(fileAbsPath));

	QMediaPlayer p;
	p.setMedia(QMediaContent(QUrl(fileAbsPath)));

	QGraphicsVideoItem* item = new QGraphicsVideoItem;
	QGraphicsScene* scene = new QGraphicsScene;
	scene->addText("TEST");
	p.setVideoOutput(item);
	scene->addItem(item);
	scene->addRect(0, 0, 100, 100, QPen(Qt::red), QBrush(Qt::red));
	item->setPos(0, 0);

	//QImage image(1920, 1080, QImage::Format_ARGB32);
	//image.fill(Qt::blue);

	//QPainter painter(&image);
	//painter.setRenderHint(QPainter::Antialiasing);
	//scene->render(&painter);

	QGraphicsView view(scene);
	//view.scene()->addItem(item);
	view.setViewport(new QOpenGLWidget);
	view.show();

	p.play();

	return a.exec();
}
