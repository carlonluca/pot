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
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/QDir>
#include <QtCore/QDebug>

#include "openmaxilplayerserviceplugin.h"
#include "openmaxilplayerservice.h"

#include "lc_logging.h"

/*------------------------------------------------------------------------------
|    OMX_PlayerServicePlugin::create
+-----------------------------------------------------------------------------*/
QMediaService* OMX_PlayerServicePlugin::create(const QString &key)
{
   LOG_VERBOSE(LOG_TAG, "Instantiating player service...");
   if (key == QLatin1String(Q_MEDIASERVICE_MEDIAPLAYER))
      return new QGstreamerPlayerService;

   LOG_WARNING(LOG_TAG, "OpenMAXIL service plugin: unsupported key: %s.", qPrintable(key));
   return NULL;
}

/*------------------------------------------------------------------------------
|    OMX_PlayerServicePlugin::release
+-----------------------------------------------------------------------------*/
void OMX_PlayerServicePlugin::release(QMediaService* service)
{
   delete service;
}

/*------------------------------------------------------------------------------
|    OMX_PlayerServicePlugin::supportedFeatures
+-----------------------------------------------------------------------------*/
QMediaServiceProviderHint::Features OMX_PlayerServicePlugin::supportedFeatures(
      const QByteArray &service) const
{
   if (service == Q_MEDIASERVICE_MEDIAPLAYER)
      return /* QMediaServiceProviderHint::StreamPlayback | */ QMediaServiceProviderHint::VideoSurface;
   else
      return QMediaServiceProviderHint::Features();
}

/*------------------------------------------------------------------------------
|    OMX_PlayerServicePlugin::hasSupport
+-----------------------------------------------------------------------------*/
QMultimedia::SupportEstimate OMX_PlayerServicePlugin::hasSupport(
      const QString& mimeType, const QStringList& codecs) const
{
   Q_UNUSED(codecs);

   // Update if needed.
   if (m_supportedMimeTypeSet.isEmpty())
      updateSupportedMimeTypes();

   // TODO: Fix this to be more precise.
   if (m_supportedMimeTypeSet.contains(mimeType))
      return QMultimedia::PreferredService;
   return QMultimedia::MaybeSupported;
}

/*------------------------------------------------------------------------------
|    OMX_PlayerServicePlugin::updateSupportedMimeType
+-----------------------------------------------------------------------------*/
void OMX_PlayerServicePlugin::updateSupportedMimeTypes() const
{
   // TODO: Add the correct list of the supported mime types.
   m_supportedMimeTypeSet.insert("video/mpeg");
   m_supportedMimeTypeSet.insert("video/mp4");
   m_supportedMimeTypeSet.insert("video/ogg");
   m_supportedMimeTypeSet.insert("video/quicktime");
   m_supportedMimeTypeSet.insert("video/webm");
   m_supportedMimeTypeSet.insert("video/x-matroska");
   m_supportedMimeTypeSet.insert("video/x-ms-wmv");
   m_supportedMimeTypeSet.insert("video/x-flv");
}

/*------------------------------------------------------------------------------
|    OMX_PlayerServicePlugin::supportedMimeTypes
+-----------------------------------------------------------------------------*/
QStringList OMX_PlayerServicePlugin::supportedMimeTypes() const
{
   if (m_supportedMimeTypeSet.isEmpty())
      updateSupportedMimeTypes();
   return m_supportedMimeTypeSet.toList();
}
