/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    09.09.2015
 *
 * Copyright (c) 2015-2017 Luca Carlon. All rights reserved.
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

#ifndef OMX_STATICCONF_H
#define OMX_STATICCONF_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QString>

/*------------------------------------------------------------------------------
|    OMX_StaticConf class
+-----------------------------------------------------------------------------*/
class OMX_StaticConf
{
public:
	static int getInterlaceMode();
	static int getInterlaceQpu();
	static int getTextureCount();
	static bool getHalfFramerate();
	static QString getOmxWatchdogFile();
	static int getOmxWatchdogPermits();
};

#endif // OMX_STATICCONF_H
