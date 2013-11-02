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
#include <lc_logging.h>

#include "QtCore/QDate"

#include "qmediametadata.h"
#include "openmaxilmetadataprovider.h"

QT_BEGIN_NAMESPACE

#if 0
struct OMX_MetaDataKeyLookup
{
   QString key;
   const char *token;
};

static const OMX_MetaDataKeyLookup omx_metaDataKeys[] = {
   {QMediaMetaData::Title, "album"},
   {QMediaMetaData::Author, "artist"}
};
#endif

/*------------------------------------------------------------------------------
|    OMX_MetaDataProvider::OMX_MetaDataProvider
+-----------------------------------------------------------------------------*/
OMX_MetaDataProvider::OMX_MetaDataProvider(OpenMAXILPlayerControl* playerControl, QObject* parent)
   :QMetaDataReaderControl(parent)
{
#if 0
   // Store in a map.
   const int count = sizeof(omx_metaDataKeys)/sizeof(OMX_MetaDataKeyLookup);
   for (int i = 0; i < count; ++i)
      m_keysMap.insert(omx_metaDataKeys[i].key, QString(omx_metaDataKeys[i].token));
#endif

   m_playerControl = playerControl;
   onUpdateRequested(playerControl->getMetaData());

   connect(m_playerControl, SIGNAL(metaDataChanged(QVariantMap)),
           this, SLOT(onUpdateRequested(QVariantMap)));
}

/*------------------------------------------------------------------------------
|    OMX_MetaDataProvider::~OMX_MetaDataProvider
+-----------------------------------------------------------------------------*/
OMX_MetaDataProvider::~OMX_MetaDataProvider()
{
}

/*------------------------------------------------------------------------------
|    OMX_MetaDataProvider::isMetaDataAvailable
+-----------------------------------------------------------------------------*/
bool OMX_MetaDataProvider::isMetaDataAvailable() const
{
   return true;
}

/*------------------------------------------------------------------------------
|    OMX_MetaDataProvider::isWritable
+-----------------------------------------------------------------------------*/
bool OMX_MetaDataProvider::isWritable() const
{
   return false;
}
/*------------------------------------------------------------------------------
|    OMX_MetaDataProvider::metaData
+-----------------------------------------------------------------------------*/
QVariant OMX_MetaDataProvider::metaData(const QString& key) const
{
   LOG_DEBUG(LOG_TAG, "MetaData request for key: %s.", qPrintable(key));
   return m_tags.value(key);
}

/*------------------------------------------------------------------------------
|    OMX_MetaDataProvider::availableMetaData
+-----------------------------------------------------------------------------*/
QStringList OMX_MetaDataProvider::availableMetaData() const
{
   LOG_DEBUG(LOG_TAG, "Available metadata requested...");
   return m_tags.keys();
}

/*------------------------------------------------------------------------------
|    OMX_MetaDataProvider::onUpdateRequested
+-----------------------------------------------------------------------------*/
void OMX_MetaDataProvider::onUpdateRequested(const QVariantMap metaData)
{
   Q_UNUSED(metaData);

   // TODO: Complete the implementation.
   LOG_VERBOSE(LOG_TAG, "Metadata update requested.");
   m_tags.clear();
   QVariantMap::const_iterator i = metaData.constBegin();
   while (i != metaData.constEnd()) {
      if (!i.key().compare("title", Qt::CaseInsensitive))
         m_tags.insert(QMediaMetaData::Title, i.value());
      else if (!i.key().compare("artist", Qt::CaseInsensitive))
         m_tags.insert(QMediaMetaData::Author, QStringList() << i.value().toString());
      else if (!i.key().compare("date", Qt::CaseInsensitive))
         m_tags.insert(QMediaMetaData::Date, QDate::fromString(i.value().toString(), Qt::ISODate));
      else if (!i.key().compare("album", Qt::CaseInsensitive))
         m_tags.insert(QMediaMetaData::AlbumTitle, i.value());
      else if (!i.key().compare("album_artist", Qt::CaseInsensitive))
         m_tags.insert(QMediaMetaData::AlbumArtist, i.value());
      ++i;
   }

   emit metaDataChanged();
}

QT_END_NAMESPACE
