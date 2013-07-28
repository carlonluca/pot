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

#ifndef QGSTREAMERMETADATAPROVIDER_H
#define QGSTREAMERMETADATAPROVIDER_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <qmetadatareadercontrol.h>
#include <qmediacontent.h>

#include "openmaxilplayercontrol.h"

QT_BEGIN_NAMESPACE

class QGstreamerPlayerSession;

/*------------------------------------------------------------------------------
|    OMX_MetaDataProvider class
+-----------------------------------------------------------------------------*/
class OMX_MetaDataProvider : public QMetaDataReaderControl
{
    Q_OBJECT
public:
    OMX_MetaDataProvider(OpenMAXILPlayerControl* playerControl, QObject* parent);
    virtual ~OMX_MetaDataProvider();

    bool isMetaDataAvailable() const;
    bool isWritable() const;

    QVariant metaData(const QString &key) const;
    QStringList availableMetaData() const;

public slots:
    void onUpdateRequested(const QVariantMap metaData);

private:
    QVariantMap m_tags;
    QMap<QByteArray, QString> m_keysMap;
    OpenMAXILPlayerControl* m_playerControl;
};

QT_END_NAMESPACE

#endif // QGSTREAMERMETADATAPROVIDER_H
