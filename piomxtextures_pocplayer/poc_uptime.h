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

#ifndef POC_UPTIME_H
#define POC_UPTIME_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QElapsedTimer>
#include <QObject>

/*------------------------------------------------------------------------------
|    POC_Uptime classs
+-----------------------------------------------------------------------------*/
class POC_Uptime : public QObject
{
	Q_OBJECT
	Q_PROPERTY(qint64 uptime READ uptime)

public:
	POC_Uptime();

	qint64 uptime() {
		return m_timer.elapsed();
	}

public slots:
	QString uptimeString();

private:
	QElapsedTimer m_timer;
};

#endif // POC_UPTIME_H
