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

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
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
#include "openmaxilvideoprobe.h"

#include "omx_logging.h"

#include <private/qmediaplaylistnavigator_p.h>
#include <qmediaplaylist.h>
#include <private/qmediaresourceset_p.h>

QT_BEGIN_NAMESPACE

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerService::OpenMAXILPlayerService
+-----------------------------------------------------------------------------*/
OpenMAXILPlayerService::OpenMAXILPlayerService(QObject *parent):
   QMediaService(parent)
 , m_videoOutput(0)
 , m_videoRenderer(0)
 , m_videoReferenceCount(0)
{
    log_verbose("Instantiating QMediaService...");

    m_control             = new OpenMAXILPlayerControl(this);
    m_metaData            = new OpenMAXILMetaDataProvider(m_control, this);
    m_streamsControl      = new OpenMAXILStreamsControl(this);
    m_availabilityControl = new OpenMAXILAvailabilityControl(this);
    m_videoRenderer       = new OpenMAXILVideoRendererControl(m_control, this);
	 m_videoProbe          = new OpenMAXILVideoProbe(this);

    m_control->setVideoRenderer(m_videoRenderer);

	 connect(m_videoRenderer, SIGNAL(frameReady(QVideoFrame)),
				m_videoProbe, SIGNAL(videoFrameProbed(QVideoFrame)));
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::~OpenMAXILPlayerControl
+-----------------------------------------------------------------------------*/
OpenMAXILPlayerService::~OpenMAXILPlayerService()
{
	log_dtor_func;
}

QMediaControl *OpenMAXILPlayerService::requestControl(const char *name)
{
    qDebug("Requesting control for %s...", name);

    if (qstrcmp(name, QMediaPlayerControl_iid) == 0)
        return m_control;

    if (qstrcmp(name, QMetaDataReaderControl_iid) == 0)
        return m_metaData;

	 if (qstrcmp(name, QMediaVideoProbeControl_iid) == 0)
		 return m_videoProbe;

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

void OpenMAXILPlayerService::releaseControl(QMediaControl *control)
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
