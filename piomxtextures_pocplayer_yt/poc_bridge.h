#ifndef POC_BRIDGE_H
#define POC_BRIDGE_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QObject>

/*------------------------------------------------------------------------------
|    POC_Bridge class
+-----------------------------------------------------------------------------*/
class POC_Bridge : public QObject
{
	Q_OBJECT
public:
	POC_Bridge(const QString& videoId, QObject* parent = 0);

	Q_INVOKABLE void onPlayerReady();

signals:
	void playVideoId(const QString& videoId);

private:
	QString m_videoId;
};

#endif // POC_BRIDGE_H
