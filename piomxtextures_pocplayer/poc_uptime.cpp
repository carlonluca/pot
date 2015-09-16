/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    04.12.2015
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
 */

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QDateTime>
#include <QString>

#include "poc_uptime.h"

/*------------------------------------------------------------------------------
|    POC_Uptime::POC_Uptime
+-----------------------------------------------------------------------------*/
POC_Uptime::POC_Uptime() :
	QObject()
{
	m_timer.start();
}

/*------------------------------------------------------------------------------
|    POC_Uptime::uptimeString
+-----------------------------------------------------------------------------*/
QString POC_Uptime::uptimeString()
{
	qint64 up = uptime();

#define MS_IN_DAY 86400000
#define MS_IN_HOUR 3600000
#define MS_IN_MIN 60000
#define MS_IN_SEC 1000

	int days = up/MS_IN_DAY;
	int hours = (up - days*MS_IN_DAY)/MS_IN_HOUR;
	int minutes = (up - days*MS_IN_DAY - hours*MS_IN_HOUR)/MS_IN_MIN;
	int seconds = (up - days*MS_IN_DAY - hours*MS_IN_HOUR - minutes*MS_IN_MIN)/MS_IN_SEC;
	int ms = up - days*MS_IN_DAY - hours*MS_IN_HOUR - minutes*MS_IN_MIN - seconds*MS_IN_SEC;

	QString str("%1 days %2:%3:%4.%5");
	str = str.arg(days).arg(hours).arg(minutes).arg(seconds).arg(ms);

	return str;
}
