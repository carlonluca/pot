/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    05.07.2015
 *
 * Copyright (c) 2016 Luca Carlon.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Luca Carlon. The name of Luca Carlon may not be
 * used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef OMX_LOGGING_H
#define OMX_LOGGING_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QString>

#define BUILD_LOG_LEVEL_INFORMATION
#include <lc_logging.h>

using namespace std;
using namespace lightlogger;

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
