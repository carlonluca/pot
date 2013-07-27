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

#ifndef QGSTREAMERPLAYERSERVICEPLUGIN_H
#define QGSTREAMERPLAYERSERVICEPLUGIN_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <qmediaserviceproviderplugin.h>
#include <QtCore/qset.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

/*------------------------------------------------------------------------------
|    OMX_PlayerServicePlugin class
+-----------------------------------------------------------------------------*/
class OMX_PlayerServicePlugin
    : public QMediaServiceProviderPlugin
    , public QMediaServiceFeaturesInterface
    , public QMediaServiceSupportedFormatsInterface
{
    Q_OBJECT
    Q_INTERFACES(QMediaServiceFeaturesInterface)
    Q_INTERFACES(QMediaServiceSupportedFormatsInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.mediaserviceproviderfactory/5.0" FILE "mediaplayer.json")
public:
    QMediaService* create(QString const& key);
    void release(QMediaService* service);

    QMediaServiceProviderHint::Features supportedFeatures(const QByteArray& service) const;

    QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList& codecs) const;
    QStringList supportedMimeTypes() const;

private:
    void updateSupportedMimeTypes() const;

    mutable QSet<QString> m_supportedMimeTypeSet;
};

QT_END_NAMESPACE

#endif // QGSTREAMERPLAYERSERVICEPLUGIN_H
