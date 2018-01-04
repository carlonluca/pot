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


/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QFile>
#include <QFileInfo>

#include "OMXCore.h"

#include "omx_watchdog.h"
#include "omx_logging.h"

#ifdef OMX_LOCK_WATCHDOG
/*------------------------------------------------------------------------------
|    OMX_WatchDog::OMX_WatchDog
+-----------------------------------------------------------------------------*/
OMX_Watchdog::OMX_Watchdog(QString watchdogFilePath, QObject* parent) :
   QObject(parent)
 , m_watchdogFile(watchdogFilePath)
{
   moveToThread(&m_thread);

	// Note that the even timer must be in a separate thread as we MUST ensure
	// its own thread does not lock.
	m_timer.moveToThread(&m_thread);
	m_timer.setSingleShot(false);
	m_timer.setInterval(1000);

	m_thread.start();

   connect(&m_timer, SIGNAL(timeout()), this, SLOT(testOmx()));

#if 0
   m_watchdogFile.remove();
#endif
}

/*------------------------------------------------------------------------------
|    OMX_WatchDog::startWatchDog
+-----------------------------------------------------------------------------*/
void OMX_Watchdog::startWatchdog()
{
	QMetaObject::invokeMethod(&m_timer, "start");
}

/*------------------------------------------------------------------------------
|    OMX_WatchDog::stopWatchDog
+-----------------------------------------------------------------------------*/
void OMX_Watchdog::stopWatchdog()
{
	QMetaObject::invokeMethod(&m_timer, "stop");
}

/*------------------------------------------------------------------------------
|    OMX_WatchDog::testOmx
+-----------------------------------------------------------------------------*/
void OMX_Watchdog::testOmx()
{
#define WATCHDOG_FILE_PATH \
   qPrintable(QFileInfo(m_watchdogFile).absoluteFilePath())

	if (!COMXCoreComponent::testOmx())
		return;

	log_verbose("Touching IPC file...");
   if (!m_watchdogFile.open(QIODevice::WriteOnly)) {
      log_warn("Cannot touch file %s.", WATCHDOG_FILE_PATH);
		return;
	}

   if (!m_watchdogFile.exists()) {
      log_warn("Could not create file %s.", WATCHDOG_FILE_PATH);
		return;
	}

   m_watchdogFile.flush();
   m_watchdogFile.close();
}
#endif // OMX_LOCK_WATCHDOG
