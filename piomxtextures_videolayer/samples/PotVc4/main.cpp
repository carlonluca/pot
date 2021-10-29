#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

	QCommandLineParser parser;
	parser.setApplicationDescription(QStringLiteral("POTVL demo app"));
	parser.addHelpOption();

	QCommandLineOption optOrientation(QStringLiteral("orientation"), QStringLiteral("[0, 1, 2, 3]"), QStringLiteral("id"));

	parser.addOption(optOrientation);
	parser.addPositionalArgument(QStringLiteral("mediapath"), QStringLiteral("Path of media file to play"));
	parser.process(app);

	QString filePath = parser.positionalArguments()[0];
	QString orientation = parser.value(QStringLiteral("orientation"));

#if 0
    QSurfaceFormat format;
    format.setStencilBufferSize(8);
    format.setDepthBufferSize(24);
    format.setRedBufferSize(5);
    format.setGreenBufferSize(6);
    format.setBlueBufferSize(5);
    format.setAlphaBufferSize(8);
#endif

    //QQmlApplicationEngine engine;
    QQuickView view;
    //view.setFormat(format);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    view.engine()->rootContext()->setContextProperty("filePath", filePath);
	view.engine()->rootContext()->setContextProperty("orientation", orientation);
    view.setSource(url);
#if QT_VERSION_MAJOR <= 5
    view.setClearBeforeRendering(true);
#endif
    view.setColor(Qt::transparent);
    view.show();

    return app.exec();
}
