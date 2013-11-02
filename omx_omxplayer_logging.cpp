/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    25.08.2013
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include "system.h"
#include "log.h"
#include "stdio_utf8.h"
#include "stat_utf8.h"
#include "utils/StdString.h"

#include "lc_logging.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
static FILE*       m_file           = NULL;
static std::string m_repeatLine     = "";
static int         m_logLevel       = LOG_LEVEL_NONE;

static pthread_mutex_t   m_log_mutex;

/*------------------------------------------------------------------------------
|    CLog::CLog
+-----------------------------------------------------------------------------*/
CLog::CLog()
{}

/*------------------------------------------------------------------------------
|    CLog::~CLog
+-----------------------------------------------------------------------------*/
CLog::~CLog()
{}

/*------------------------------------------------------------------------------
|    CLog::Close
+-----------------------------------------------------------------------------*/
void CLog::Close()
{
  if (m_file)
  {
    fclose(m_file);
    m_file = NULL;
  }
  m_repeatLine.clear();
  pthread_mutex_destroy(&m_log_mutex);
}

// ones we use in the code
#define LOGDEBUG   0
#define LOGINFO    1
#define LOGNOTICE  2
#define LOGWARNING 3
#define LOGERROR   4
#define LOGSEVERE  5
#define LOGFATAL   6
#define LOGNONE    7

/*------------------------------------------------------------------------------
|    CLog::Log
+-----------------------------------------------------------------------------*/
void CLog::Log(int loglevel, const char *format, ... )
{
#ifdef ENABLE_OMXPLAYER_LOGS
   if (loglevel < m_logLevel)
      return;

   va_list args;
   va_start(args, format);
   switch (loglevel) {
   case LOGDEBUG:
      log_debug_v(format, args); break;
   case LOGINFO:
      log_verbose_v(format, args); break;
   case LOGNOTICE:
      log_info_v(format, args); break;
   case LOGWARNING:
      log_warn_v(format, args); break;
   case LOGERROR:
      log_err_v(format, args); break;
   case LOGSEVERE:
      log_err_v(format, args); break;
   case LOGFATAL:
      log_critical_v(format, args); break;
   default:
      log_verbose_v(format, args); break;
   }
   va_end(args);
#else
   (void)loglevel;
   (void)format;
#endif
}

/*------------------------------------------------------------------------------
|    CLog::Init
+-----------------------------------------------------------------------------*/
bool CLog::Init(const char* path)
{
  return true;
}

/*------------------------------------------------------------------------------
|    CLog::MemDump
+-----------------------------------------------------------------------------*/
void CLog::MemDump(char *pData, int length)
{
  if (m_logLevel > LOG_LEVEL_NONE) {
  Log(LOGDEBUG, "MEM_DUMP: Dumping from %p", pData);
  for (int i = 0; i < length; i+=16)
  {
    CStdString strLine;
    strLine.Format("MEM_DUMP: %04x ", i);
    char *alpha = pData;
    for (int k=0; k < 4 && i + 4*k < length; k++)
    {
      for (int j=0; j < 4 && i + 4*k + j < length; j++)
      {
        CStdString strFormat;
        strFormat.Format(" %02x", *pData++);
        strLine += strFormat;
      }
      strLine += " ";
    }
    // pad with spaces
    while (strLine.size() < 13*4 + 16)
      strLine += " ";
    for (int j=0; j < 16 && i + j < length; j++)
    {
      if (*alpha > 31)
        strLine += *alpha;
      else
        strLine += '.';
      alpha++;
    }
    Log(LOGDEBUG, "%s", strLine.c_str());
  }
  }
}

/*------------------------------------------------------------------------------
|    CLog::SetLogLevel
+-----------------------------------------------------------------------------*/
void CLog::SetLogLevel(int level)
{
  if(m_logLevel > LOG_LEVEL_NONE)
    CLog::Log(LOGNOTICE, "Log level changed to %d", m_logLevel);
  m_logLevel = level;
}

/*------------------------------------------------------------------------------
|    CLog::GetLogLevel
+-----------------------------------------------------------------------------*/
int CLog::GetLogLevel()
{
  return m_logLevel;
}

/*------------------------------------------------------------------------------
|    CLog::OutputDebugString
+-----------------------------------------------------------------------------*/
void CLog::OutputDebugString(const std::string& line)
{
   return;
}
