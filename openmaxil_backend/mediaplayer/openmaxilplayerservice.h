/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

#ifndef QGSTREAMERPLAYERSERVICE_H
#define QGSTREAMERPLAYERSERVICE_H

#include <QtCore/qobject.h>
#include <QtCore/qiodevice.h>

#include <qmediaservice.h>

QT_BEGIN_NAMESPACE
class QMediaPlayerControl;
class QMediaPlaylist;
class QMediaPlaylistNavigator;

class QGstreamerMetaData;
class OpenMAXILPlayerControl;
class QGstreamerPlayerSession;
class QGstreamerMetaDataProvider;
class QGstreamerStreamsControl;
class QGstreamerVideoRenderer;
class QGstreamerVideoOverlay;
class QGstreamerVideoWidgetControl;
class OpenMAXILAvailabilityControl;
class OpenMAXILVideoRendererControl;

class QGstreamerPlayerService : public QMediaService
{
    Q_OBJECT
public:
    QGstreamerPlayerService(QObject *parent = 0);
    ~QGstreamerPlayerService();

    QMediaControl *requestControl(const char *name);
    void releaseControl(QMediaControl *control);

private:
    OpenMAXILPlayerControl *m_control;
    QGstreamerMetaDataProvider *m_metaData;
    QGstreamerStreamsControl *m_streamsControl;
    OpenMAXILAvailabilityControl *m_availabilityControl;

    QMediaControl *m_videoOutput;
#if 0
    QMediaControl *m_videoRenderer;
#endif
    OpenMAXILVideoRendererControl* m_videoRenderer;
#if defined(HAVE_XVIDEO) && defined(HAVE_WIDGETS)
    QMediaControl *m_videoWindow;
    QMediaControl *m_videoWidget;
#endif
    int m_videoReferenceCount;
};

QT_END_NAMESPACE

#endif
