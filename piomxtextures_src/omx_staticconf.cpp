/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    09.09.2015
 *
 * Copyright (c) 2015 Luca Carlon. All rights reserved.
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
 * along with PiOmxTextures.  If not, see <http://www.gnu.org/licenses/>.
 */

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QByteArray>

#include <mutex>

#include "omx_logging.h"
#include "omx_staticconf.h"

static std::once_flag flag1;
static std::once_flag flag2;
static std::once_flag flag3;
static std::once_flag flag4;
static std::once_flag flag5;

/*------------------------------------------------------------------------------
|    OMX_StaticConf::getHalfFramerate
+-----------------------------------------------------------------------------*/
bool OMX_StaticConf::getHalfFramerate()
{
	static bool halfFramerate = false;
	std::call_once(flag1, []() {
		QByteArray ba = qgetenv("POT_HALF_FRAMERATE_MODE");
		if (ba.isEmpty() || ba.isNull())
			return;
		bool convOk;
		int hf = ba.toInt(&convOk);
		if (convOk && (hf == 0 || hf == 1))
			halfFramerate = hf;

		log_verbose("Setting half framerate to %d.", halfFramerate);
	});

	return halfFramerate;
}

/*------------------------------------------------------------------------------
|    OMX_StaticConf::getTextureCount
+-----------------------------------------------------------------------------*/
int OMX_StaticConf::getTextureCount()
{
	static int textureCount = 4;
	std::call_once(flag3, []() {
		QByteArray ba = qgetenv("POT_TEXTURE_COUNT");
		if (ba.isEmpty() || ba.isNull())
			return;
		bool convOk;
		int tc = ba.toInt(&convOk);
		if (convOk && tc > 1)
			textureCount = tc;

		log_verbose("Setting texture count to %d.", textureCount);
	});

	return textureCount;
}

/*------------------------------------------------------------------------------
|    OMX_StaticConf::getInterlaceMode
+-----------------------------------------------------------------------------*/
int OMX_StaticConf::getInterlaceMode()
{
	static int interlaceMode = 0; // Off.
	std::call_once(flag2, []() {
		QByteArray ba = qgetenv("POT_DEINTERLACE_MODE");
		if (ba.isEmpty() || ba.isNull())
			return;
		bool convOk;
		int im = ba.toInt(&convOk);
		if (convOk && (im >= 0 && im < 3))
			interlaceMode = im;

		log_verbose("Setting interlace mode to %d.", interlaceMode);
	});

	return interlaceMode;
}

/*------------------------------------------------------------------------------
|    OMX_StaticConf::getInterlaceQpu
+-----------------------------------------------------------------------------*/
int OMX_StaticConf::getInterlaceQpu()
{
	static int interlaceQpu = 1;
	std::call_once(flag4, [] {
		QByteArray ba = qgetenv("POT_DEINTERLACE_QPU");
		if (ba.isEmpty() || ba.isNull())
			return;
		bool convOk;
		int qpu = ba.toInt(&convOk);
		if (convOk && (qpu != 0 && qpu != 1))
			interlaceQpu = qpu;

		log_verbose("Setting qpu interlace to %d.", qpu);
	});

	return interlaceQpu;
}

/*------------------------------------------------------------------------------
|    OMX_StaticConf::getOmxWatchdogFile
+-----------------------------------------------------------------------------*/
QString OMX_StaticConf::getOmxWatchdogFile()
{
   static QString watchdogFile = QString();
   std::call_once(flag5, [] {
      QByteArray ba = qgetenv("OMX_HEALTHY");
      if (ba.isEmpty() || ba.isNull())
         return;
      watchdogFile = QString(ba);

      log_verbose("OMX_HEALTHY is %s", qPrintable(watchdogFile));
   });

   return watchdogFile;
}
