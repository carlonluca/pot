/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    04.14.2013
 *
 * Copyright (c) 2012 Luca Carlon. All rights reserved.
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

#include "openmaxilmetadataprovider.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

struct QGstreamerMetaDataKeyLookup
{
    QString key;
    const char *token;
};

#if 0
static const QGstreamerMetaDataKeyLookup qt_gstreamerMetaDataKeys[] =
{
    { QMediaMetaData::Title, GST_TAG_TITLE },
    //{ QMediaMetaData::SubTitle, 0 },
    //{ QMediaMetaData::Author, 0 },
    { QMediaMetaData::Comment, GST_TAG_COMMENT },
    { QMediaMetaData::Description, GST_TAG_DESCRIPTION },
    //{ QMediaMetaData::Category, 0 },
    { QMediaMetaData::Genre, GST_TAG_GENRE },
    { QMediaMetaData::Year, "year" },
    //{ QMediaMetaData::UserRating, 0 },

    { QMediaMetaData::Language, GST_TAG_LANGUAGE_CODE },

    { QMediaMetaData::Publisher, GST_TAG_ORGANIZATION },
    { QMediaMetaData::Copyright, GST_TAG_COPYRIGHT },
    //{ QMediaMetaData::ParentalRating, 0 },
    //{ QMediaMetaData::RatingOrganisation, 0 },

    // Media
    //{ QMediaMetaData::Size, 0 },
    //{ QMediaMetaData::MediaType, 0 },
    { QMediaMetaData::Duration, GST_TAG_DURATION },

    // Audio
    { QMediaMetaData::AudioBitRate, GST_TAG_BITRATE },
    { QMediaMetaData::AudioCodec, GST_TAG_AUDIO_CODEC },
    //{ QMediaMetaData::ChannelCount, 0 },
    //{ QMediaMetaData::SampleRate, 0 },

    // Music
    { QMediaMetaData::AlbumTitle, GST_TAG_ALBUM },
    { QMediaMetaData::AlbumArtist,  GST_TAG_ARTIST},
    { QMediaMetaData::ContributingArtist, GST_TAG_PERFORMER },
#if (GST_VERSION_MAJOR >= 0) && (GST_VERSION_MINOR >= 10) && (GST_VERSION_MICRO >= 19)
    { QMediaMetaData::Composer, GST_TAG_COMPOSER },
#endif
    //{ QMediaMetaData::Conductor, 0 },
    //{ QMediaMetaData::Lyrics, 0 },
    //{ QMediaMetaData::Mood, 0 },
    { QMediaMetaData::TrackNumber, GST_TAG_TRACK_NUMBER },

    //{ QMediaMetaData::CoverArtUrlSmall, 0 },
    //{ QMediaMetaData::CoverArtUrlLarge, 0 },

    // Image/Video
    { QMediaMetaData::Resolution, "resolution" },
    { QMediaMetaData::PixelAspectRatio, "pixel-aspect-ratio" },

    // Video
    //{ QMediaMetaData::VideoFrameRate, 0 },
    //{ QMediaMetaData::VideoBitRate, 0 },
    { QMediaMetaData::VideoCodec, GST_TAG_VIDEO_CODEC },

    //{ QMediaMetaData::PosterUrl, 0 },

    // Movie
    //{ QMediaMetaData::ChapterNumber, 0 },
    //{ QMediaMetaData::Director, 0 },
    { QMediaMetaData::LeadPerformer, GST_TAG_PERFORMER },
    //{ QMediaMetaData::Writer, 0 },

    // Photos
    //{ QMediaMetaData::CameraManufacturer, 0 },
    //{ QMediaMetaData::CameraModel, 0 },
    //{ QMediaMetaData::Event, 0 },
    //{ QMediaMetaData::Subject, 0 }
};
#endif

QGstreamerMetaDataProvider::QGstreamerMetaDataProvider(QObject *parent)
    :QMetaDataReaderControl(parent)
{
#if 0
    const int count = sizeof(qt_gstreamerMetaDataKeys) / sizeof(QGstreamerMetaDataKeyLookup);
    for (int i = 0; i < count; ++i) {
        m_keysMap[QByteArray(qt_gstreamerMetaDataKeys[i].token)] = qt_gstreamerMetaDataKeys[i].key;
    }
#endif
}

QGstreamerMetaDataProvider::~QGstreamerMetaDataProvider()
{
}

bool QGstreamerMetaDataProvider::isMetaDataAvailable() const
{
    return false;
}

bool QGstreamerMetaDataProvider::isWritable() const
{
    return false;
}

QVariant QGstreamerMetaDataProvider::metaData(const QString &key) const
{
    return m_tags.value(key);
}

QStringList QGstreamerMetaDataProvider::availableMetaData() const
{
    return m_tags.keys();
}

void QGstreamerMetaDataProvider::updateTags()
{
   // TODO: Implement.
#if 0
    QVariantMap oldTags = m_tags;
    m_tags.clear();

    QSet<QString> allTags = QSet<QString>::fromList(m_tags.keys());

    QMapIterator<QByteArray ,QVariant> i(m_session->tags());
    while (i.hasNext()) {
         i.next();
         //use gstreamer native keys for elements not in m_keysMap
         QString key = m_keysMap.value(i.key(), i.key());
         m_tags[key] = i.value();
         allTags.insert(key);
    }

    bool changed = false;
    foreach (const QString &key, allTags) {
        const QVariant value = m_tags.value(key);
        if (value != oldTags.value(key)) {
            changed = true;
            emit metaDataChanged(key, value);
        }
    }

    if (changed)
        emit metaDataChanged();
#endif
}

QT_END_NAMESPACE
