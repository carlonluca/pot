/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.28.2013
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef POC_QMLUTILS_H
#define POC_QMLUTILS_H

/*----------------------------------------------------------------------
|    includes
+---------------------------------------------------------------------*/
#include <QObject>
#include <QStringList>

/*----------------------------------------------------------------------
|    POC_QMLUtils class
+---------------------------------------------------------------------*/
class POC_QMLUtils : public QObject
{
   Q_OBJECT
public:
   explicit POC_QMLUtils(QObject* parent = 0);

   Q_INVOKABLE QString getHomeDir();
   Q_INVOKABLE QString getPathFromUri(QString uri);
   Q_INVOKABLE QString getFileExtension(QString file);

   // File methods.
   Q_INVOKABLE bool isSupportedImage(QString file);
   Q_INVOKABLE bool isSupportedVideo(QString file);
   Q_INVOKABLE bool isSupportedAudio(QString file);

   // Gallery methods.
   Q_INVOKABLE QString getNextImage(QString imageAbsPath);
   Q_INVOKABLE QString getPrevImage(QString imageAbsPath);
   Q_INVOKABLE QStringList getSupportedImageExtensions();
};

#endif // POC_QMLUTILS_H
