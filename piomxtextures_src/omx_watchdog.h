/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    10.22.2015
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

#ifndef OMX_WATCHDOG_H
#define OMX_WATCHDOG_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QTimer>
#include <QThread>

/*------------------------------------------------------------------------------
|    OMX_WatchDog class
+-----------------------------------------------------------------------------*/
#ifdef OMX_LOCK_WATCHDOG
class OMX_Watchdog : public QObject
{
	Q_OBJECT
public:
	OMX_Watchdog(QObject* parent = 0);
	virtual ~OMX_Watchdog() {}

public slots:
	void startWatchdog();
	void stopWatchdog();
	void testOmx();

private:
	QTimer m_timer;
	QThread m_thread;
};
#endif // OMX_LOCK_WATCHDOG
#endif // OMX_WATCHDOG_H
