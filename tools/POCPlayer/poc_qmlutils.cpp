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

/*----------------------------------------------------------------------
|    includes
+---------------------------------------------------------------------*/
#include <QDir>

#include "poc_qmlutils.h"
#include "poc_utils.h"

/*----------------------------------------------------------------------
|    POC_QMLUtils::POC_QMLUtils
+---------------------------------------------------------------------*/
POC_QMLUtils::POC_QMLUtils(QObject* parent) :
   QObject(parent)
{
   // Do nothing.
}

/*----------------------------------------------------------------------
|    POC_QMLUtils::getHomeDir
+---------------------------------------------------------------------*/
QString POC_QMLUtils::getHomeDir()
{
   return QDir::homePath();
}

/*----------------------------------------------------------------------
|    POC_QMLUtils::getPathFromUri
+---------------------------------------------------------------------*/
/**
 * @brief POC_QMLUtils::getPathFromUri Returns the path given a local (file://)
 * URI. Otherwise a null string is returned.
 * @param uri
 * @return
 */
QString POC_QMLUtils::getPathFromUri(QString uri)
{
   // URLs should be case sensitive: http://www.w3.org/TR/WD-html40-970708/htmlweb.html.
   if (!uri.startsWith("file://", Qt::CaseSensitive))
      return QString();

   return uri.mid(7, uri.size() - 7);
}

/*----------------------------------------------------------------------
|    POC_QMLUtils::getFileExtension
+---------------------------------------------------------------------*/
/**
 * @brief POC_QMLUtils::getFileExtension Returns the file extension. file can be abs or
 * relative.
 * @param file
 * @return
 */
QString POC_QMLUtils::getFileExtension(QString file)
{
   return POC_Utils::getFileExtension(file);
}

/*----------------------------------------------------------------------
|    POC_Utils::isSupportedAudio
+---------------------------------------------------------------------*/
bool POC_QMLUtils::isSupportedAudio(QString file)
{
   return POC_Utils::isSupportedAudio(file);
}

/*----------------------------------------------------------------------
|    POC_Utils::isSupportedVideo
+---------------------------------------------------------------------*/
bool POC_QMLUtils::isSupportedVideo(QString file)
{
   return POC_Utils::isSupportedVideo(file);
}

/*----------------------------------------------------------------------
|    POC_Utils::isSupportedImage
+---------------------------------------------------------------------*/
bool POC_QMLUtils::isSupportedImage(QString file)
{
   return POC_Utils::isSupportedImage(file);
}

/*----------------------------------------------------------------------
|    POC_QMLUtils::getNextImage
+---------------------------------------------------------------------*/
/**
 * @brief POC_QMLUtils::getPrevImage Returns the alphabetically next image in the
 * same directory.
 * @param imageAbsPath
 * @return
 */
QString POC_QMLUtils::getNextImage(QString imageAbsPath)
{
   QFileInfo fileInfo(imageAbsPath);
   QDir dir = fileInfo.absoluteDir();

   // List the files in the directory.
   QFileInfoList files = dir.entryInfoList(QStringList() << "*.jpg", QDir::Files, QDir::Name);
   int index = files.lastIndexOf(QFileInfo(imageAbsPath));
   if (index < -1)
      return QString();

   index = (index + 1)%files.size();
   return files.at(index).absoluteFilePath();
}

/*----------------------------------------------------------------------
|    POC_QMLUtils::getPrevImage
+---------------------------------------------------------------------*/
/**
 * @brief POC_QMLUtils::getPrevImage Returns the alphabetically previous image in the
 * same directory.
 * @param imageAbsPath
 * @return
 */
QString POC_QMLUtils::getPrevImage(QString imageAbsPath)
{
   QFileInfo fileInfo(imageAbsPath);
   QDir dir = fileInfo.absoluteDir();

   // List the files in the directory.
   QFileInfoList files = dir.entryInfoList(QStringList() << "*.jpg", QDir::Files, QDir::Name);
   int index = files.lastIndexOf(QFileInfo(imageAbsPath));
   if (index < -1)
      return QString();

   index = (index <= 0) ? files.size() - 1 : index - 1;
   return files.at(index).absoluteFilePath();
}

/*----------------------------------------------------------------------
|    POC_QMLUtils::getSupportedImageExtensions
+---------------------------------------------------------------------*/
/**
 * @brief POC_QMLUtils::getSupportedImageExtensions Returns a list of the supported image
 * extensions on the current platform.
 * @return
 */
QStringList POC_QMLUtils::getSupportedImageExtensions()
{
   return POC_Utils::getSupportedImageExtensions();
}
