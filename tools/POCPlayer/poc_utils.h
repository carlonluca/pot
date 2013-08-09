/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    08.09.2013
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

#ifndef POC_UTILS_H
#define POC_UTILS_H

/*----------------------------------------------------------------------
|    includes
+---------------------------------------------------------------------*/
#include <QHash>
#include <QString>

/*----------------------------------------------------------------------
|    POC_Utils class
+---------------------------------------------------------------------*/
class POC_Utils
{
public:
   static const QStringList& getSupportedImageFormats();
   static const QStringList& getSupportedImageExtensions();

   static const QStringList& getSupportedVideoExtensions();

   static bool isSupportedVideo(QString file);
   static bool isSupportedImage(QString file);
   static bool isSupportedAudio(QString file);

   static void listSupportedImageFormats();
   static void listSupportedImageExtensions();

   static const QHash<QString, QString>& getMimeToExtMap();

   static QString getFileExtension(const QString& file);
};

#endif // POC_UTILS_H
