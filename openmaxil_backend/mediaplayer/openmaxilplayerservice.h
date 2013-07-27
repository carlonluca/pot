/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    04.14.2013
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
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
class OMX_MetaDataProvider;
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
    OMX_MetaDataProvider *m_metaData;
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
