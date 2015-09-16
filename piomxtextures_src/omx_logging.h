/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    05.07.2015
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

#ifndef OMX_LOGGING_H
#define OMX_LOGGING_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QString>

#include <lc_logging.h>

using namespace std;

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#define ENABLE_DTOR_LOGS
#ifdef ENABLE_DTOR_LOGS
#define log_dtor(f, ...) \
	log_formatted(LC_LOG_ATTR_BLINK, LC_LOG_COL_WHITE, f, ##__VA_ARGS__)
#else
#define log_dtor(...)
#endif // ENABLE_DTOR_LOGS

#define log_dtor_func \
   log_dtor("%s", __PRETTY_FUNCTION__)

#ifdef ENABLE_LOG_DEBUG
#define logi_debug(f, ...)               \
   {                                     \
      QString p = QString::asprintf("[%p] ", this);  \
      QString l = p + f;                  \
      log_debug(l.toLocal8Bit().constData(), __VA_ARGS__); \
   }
#define logi_debug_func                                 \
   {                                                    \
      QString p = QString::asprintf("[%p] %s", this, Q_FUNC_INFO);  \
      log_debug(p.toLocal8Bit().constData());                             \
   }

#else
#define logi_debug
#define logi_debug_func
#endif

#endif // OMX_LOGGING_H
