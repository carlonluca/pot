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

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

#if defined(HAVE_WIDGETS)
#include <QtWidgets/qwidget.h>
#endif

#include "openmaxilplayerservice.h"
#include "openmaxilplayercontrol.h"
#include "openmaxilmetadataprovider.h"
#include "openmaxilavailabilitycontrol.h"
#include "openmaxilvideorenderercontrol.h"
#include "openmaxilstreamscontrol.h"

#include <private/qmediaplaylistnavigator_p.h>
#include <qmediaplaylist.h>
#include <private/qmediaresourceset_p.h>

QT_BEGIN_NAMESPACE

QGstreamerPlayerService::QGstreamerPlayerService(QObject *parent):
     QMediaService(parent)
     , m_videoOutput(0)
     , m_videoRenderer(0)
#if defined(HAVE_XVIDEO) && defined(HAVE_WIDGETS)
     , m_videoWindow(0)
     , m_videoWidget(0)
#endif
     , m_videoReferenceCount(0)
{
    qDebug("Instantiating QMediaService...");

    m_control             = new OpenMAXILPlayerControl(this);
    m_metaData            = new OMX_MetaDataProvider(m_control, this);
    m_streamsControl      = new QGstreamerStreamsControl(this);
    m_availabilityControl = new OpenMAXILAvailabilityControl(this);

#if defined(Q_WS_MAEMO_6) && defined(__arm__)
    m_videoRenderer = new QGstreamerGLTextureRenderer(this);
#else
    //m_videoRenderer = new QGstreamerVideoRenderer(this);
    m_videoRenderer = new OpenMAXILVideoRendererControl(this);

    // Connect directly to avoid deadlock.
    connect(m_control, SIGNAL(textureInvalidated()),
            m_videoRenderer, SLOT(onTextureInvalidated()), Qt::DirectConnection);
    connect(m_control, SIGNAL(textureReady(const OMX_TextureData*)),
            m_videoRenderer, SLOT(onTextureReady(const OMX_TextureData*)), Qt::DirectConnection);
#endif

#if defined(HAVE_XVIDEO) && defined(HAVE_WIDGETS)
#ifdef Q_WS_MAEMO_6
    m_videoWindow = new QGstreamerVideoWindow(this, "omapxvsink");
#else
    m_videoWindow = new QGstreamerVideoOverlay(this);
#endif
    m_videoWidget = new QGstreamerVideoWidgetControl(this);
#endif
}

QGstreamerPlayerService::~QGstreamerPlayerService()
{
}

QMediaControl *QGstreamerPlayerService::requestControl(const char *name)
{
    qDebug("Requesting control for %s...", name);

    if (qstrcmp(name, QMediaPlayerControl_iid) == 0)
        return m_control;

    if (qstrcmp(name, QMetaDataReaderControl_iid) == 0)
        return m_metaData;

#if 0
    if (qstrcmp(name, QMediaStreamsControl_iid) == 0)
        return 0;

    if (qstrcmp(name, QMediaAvailabilityControl_iid) == 0)
        return 0;

    if (qstrcmp(name, QMediaVideoProbeControl_iid) == 0) {
        return 0;
        //if (m_session) {
        //    QGstreamerVideoProbeControl *probe = new QGstreamerVideoProbeControl(this);
        //    increaseVideoRef();
        //    m_session->addProbe(probe);
        //    return probe;
        //}
        //return 0;
    }

    if (qstrcmp(name, QMediaAudioProbeControl_iid) == 0) {
        //if (m_session) {
        //    QGstreamerAudioProbeControl *probe = new QGstreamerAudioProbeControl(this);
        //    m_session->addProbe(probe);
        //    return probe;
        //}
        return 0;
    }
#endif

    if (qstrcmp(name, QVideoRendererControl_iid) == 0)
       return m_videoRenderer;

#if 0
    if (!m_videoOutput) {
        if (qstrcmp(name, QVideoRendererControl_iid) == 0)
            m_videoOutput = m_videoRenderer;
#if defined(HAVE_XVIDEO) && defined(HAVE_WIDGETS)
        else if (qstrcmp(name, QVideoWidgetControl_iid) == 0)
            m_videoOutput = m_videoWidget;
        else  if (qstrcmp(name, QVideoWindowControl_iid) == 0)
            m_videoOutput = m_videoWindow;
#endif

        if (m_videoOutput) {
            increaseVideoRef();
            m_control->setVideoOutput(m_videoOutput);
            return m_videoOutput;
        }
    }
#endif

    return 0;
}

void QGstreamerPlayerService::releaseControl(QMediaControl *control)
{
#if 0
    if (control == m_videoOutput) {
        m_videoOutput = 0;
        m_control->setVideoOutput(0);
        decreaseVideoRef();
    }

    QGstreamerVideoProbeControl* videoProbe = qobject_cast<QGstreamerVideoProbeControl*>(control);
    if (videoProbe) {
        if (m_session) {
            m_session->removeProbe(videoProbe);
            decreaseVideoRef();
        }
        delete videoProbe;
        return;
    }

    QGstreamerAudioProbeControl* audioProbe = qobject_cast<QGstreamerAudioProbeControl*>(control);
    if (audioProbe) {
        if (m_session)
            m_session->removeProbe(audioProbe);
        delete audioProbe;
        return;
    }
#endif
}

QT_END_NAMESPACE
