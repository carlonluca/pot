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

/*----------------------------------------------------------------------
|    includes
+---------------------------------------------------------------------*/
#include <QImageReader>
#include <QMutex>
#include <QFileInfo>

#include "poc_utils.h"

/*----------------------------------------------------------------------
|    POC_Utils::supportedImageFormats
+---------------------------------------------------------------------*/
const QStringList& POC_Utils::getSupportedImageFormats()
{
   static QStringList supported;
   if (supported.isEmpty()) {
      QList<QByteArray> mimes = QImageReader::supportedMimeTypes();
      foreach (QByteArray a, mimes)
         supported.append(QString(a));
   }

   return supported;
}

/*----------------------------------------------------------------------
|    POC_Utils::getSupportedImageExtensions
+---------------------------------------------------------------------*/
const QStringList& POC_Utils::getSupportedImageExtensions()
{
   static QStringList ret;
   if (!ret.isEmpty())
      return ret;

   const QStringList supported = POC_Utils::getSupportedImageFormats();

   foreach (QString a, supported) {
      QStringList mimes = getMimeToExtMap().values(a);
      if (mimes.isEmpty()) {
         qDebug("Can't find extension for %s.", qPrintable(a));
         continue;
      }

      foreach (QString b, mimes)
         ret.append(b.toLocal8Bit());
   }

   return ret;
}

/*----------------------------------------------------------------------
|    POC_Utils::getSupportedVideoExtensions
+---------------------------------------------------------------------*/
const QStringList& POC_Utils::getSupportedVideoExtensions()
{
   static QStringList supported;
   if (supported.isEmpty()) {
      supported.append("mp4");
      supported.append("mov");
      supported.append("mkv");

      // TODO: Add the other supported extensions.
   }

   return supported;
}

/*----------------------------------------------------------------------
|    POC_Utils::getSupportedAudioExtensions
+---------------------------------------------------------------------*/
const QStringList& POC_Utils::getSupportedAudioExtensions()
{
   static QStringList supported;
   if (supported.isEmpty()) {
      supported.append("mp3");
      supported.append("ogg");
      supported.append("wma");

      // TODO: Add the other supported extensions.
   }

   return supported;
}

/*----------------------------------------------------------------------
|    POC_Utils::getFilterFromExtensions
+---------------------------------------------------------------------*/
const QStringList POC_Utils::getFilterFromExts(const QStringList& extensions)
{
   QStringList ret;
   foreach (QString a, extensions)
      ret.append(QString("*.") + a);
   return ret;
}

/*----------------------------------------------------------------------
|    POC_Utils::isSupportedAudio
+---------------------------------------------------------------------*/
bool POC_Utils::isSupportedAudio(QString file)
{
   QString ext = getFileExtension(file);
   if (ext.isNull())
      return false;

   QStringList supported = getSupportedAudioExtensions();
   return supported.contains(ext);
}

/*----------------------------------------------------------------------
|    POC_Utils::isSupportedVideo
+---------------------------------------------------------------------*/
bool POC_Utils::isSupportedVideo(QString file)
{
   QString ext = getFileExtension(file);
   if (ext.isNull())
      return false;

   QStringList supported = getSupportedVideoExtensions();
   return supported.contains(ext);
}

/*----------------------------------------------------------------------
|    POC_Utils::isSupportedImage
+---------------------------------------------------------------------*/
bool POC_Utils::isSupportedImage(QString file)
{
   QString ext = getFileExtension(file);
   if (ext.isNull())
      return false;

   QStringList supported = getSupportedImageExtensions();
   return supported.contains(ext);
}

/*----------------------------------------------------------------------
|    POC_Utils::listSupportedImageFormats
+---------------------------------------------------------------------*/
void POC_Utils::listSupportedImageFormats()
{
   QList<QString> supported = getSupportedImageFormats();
   foreach (QString s, supported)
      qDebug("Supported format: %s.", qPrintable(s));
}

/*----------------------------------------------------------------------
|    POC_Utils::listSupportedImageExtensions
+---------------------------------------------------------------------*/
void POC_Utils::listSupportedImageExtensions()
{
   QList<QString> supported = getSupportedImageExtensions();
   foreach (QString s, supported)
      qDebug("Supported extension: %s.", qPrintable(s));
}

/*----------------------------------------------------------------------
|    POC_Utils::getMimeToExtMap
+---------------------------------------------------------------------*/
const QHash<QString, QString>& POC_Utils::getMimeToExtMap()
{
   static QMutex mutex;
   QMutexLocker locker(&mutex);

   static QHash<QString, QString> map;
   if (map.isEmpty()) {
      map.insertMulti("image/bmp", "bmp");
      map.insertMulti("image/cgm", "cgm");
      map.insertMulti("image/g3fax", "g3");
      map.insertMulti("image/gif", "gif");
      map.insertMulti("image/ief", "ief");
      map.insertMulti("image/jpeg", "jpeg");
      map.insertMulti("image/jpeg", "jpg");
      map.insertMulti("image/jpeg", "jpe");
      map.insertMulti("image/ktx", "ktx");
      map.insertMulti("image/png", "png");
      map.insertMulti("image/prs.btif", "btif");
      map.insertMulti("image/sgi", "sgi");
      map.insertMulti("image/svg+xml", "svgz");
      map.insertMulti("image/svg+xml", "svg");
      map.insertMulti("image/tiff", "tiff");
      map.insertMulti("image/tiff", "tif");
      map.insertMulti("image/vnd.adobe.photoshop", "psd");
      map.insertMulti("image/vnd.dece.graphic", "uvi");
      map.insertMulti("image/vnd.dece.graphic", "uvvi");
      map.insertMulti("image/vnd.dece.graphic", "uvg");
      map.insertMulti("image/vnd.dece.graphic", "uvvg");
      map.insertMulti("image/vnd.dvb.subtitle", "sub");
      map.insertMulti("image/vnd.djvu", "djvu djv");
      map.insertMulti("image/vnd.djvu", "djv");
      map.insertMulti("image/vnd.dwg", "dwg");
      map.insertMulti("image/vnd.dxf", "dxf");
      map.insertMulti("image/vnd.fastbidsheet", "fbs");
      map.insertMulti("image/vnd.fpx", "fpx");
      map.insertMulti("image/vnd.fst", "fst");
      map.insertMulti("image/vnd.fujixerox.edmics-mmr", "mmr");
      map.insertMulti("image/vnd.fujixerox.edmics-rlc", "rlc");
      map.insertMulti("image/vnd.ms-modi", "mdi");
      map.insertMulti("image/vnd.ms-photo", "wdp");
      map.insertMulti("image/vnd.net-fpx", "npx");
      map.insertMulti("image/vnd.wap.wbmp", "wbmp");
      map.insertMulti("image/vnd.xiff", "xif");
      map.insertMulti("image/webp", "webp");
      map.insertMulti("image/x-3ds", "3ds");
      map.insertMulti("image/x-cmu-raster", "ras");
      map.insertMulti("image/x-cmx", "cmx");
      map.insertMulti("image/x-freehand", "fh");
      map.insertMulti("image/x-freehand", "fhc");
      map.insertMulti("image/x-freehand", "fh4");
      map.insertMulti("image/x-freehand", "fh5");
      map.insertMulti("image/x-freehand", "fh7");
      map.insertMulti("image/x-icon", "ico");
      map.insertMulti("image/x-mrsid-image", "sid");
      map.insertMulti("image/x-pcx", "pcx");
      map.insertMulti("image/x-pict", "pct");
      map.insertMulti("image/x-pict", "pic");
      map.insertMulti("image/x-portable-anymap", "pnm");
      map.insertMulti("image/x-portable-bitmap", "pbm");
      map.insertMulti("image/x-portable-graymap", "pgm");
      map.insertMulti("image/x-portable-pixmap", "ppm");
      map.insertMulti("image/x-rgb", "rgb");
      map.insertMulti("image/x-tga", "tga");
      map.insertMulti("image/x-xbitmap", "xbm");
      map.insertMulti("image/x-xpixmap", "xpm");
      map.insertMulti("image/x-xwindowdump", "xwd");
   }

   return map;
}

/*----------------------------------------------------------------------
|    POC_Utils::getFileExtension
+---------------------------------------------------------------------*/
QString POC_Utils::getFileExtension(const QString& file)
{
   QFileInfo fileInfo(file);
   return fileInfo.suffix();
}
