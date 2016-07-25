/*
 *      Copyright (C) 2005-2009 Team XBMC
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

#include "RBP.h"
#include "utils/log.h"

#define CLASSNAME "CRBP"

int CRBP::m_refcount = 0;
QMutex CRBP::m_mutex;

CRBP::CRBP()
{
  m_initialized = false;
  m_DllBcmHost = new DllBcmHost();
}

CRBP::~CRBP()
{
  Deinitialize();
  delete m_DllBcmHost;
}

bool CRBP::Initialize()
{
  QMutexLocker l(&m_mutex);
  if (m_refcount > 0)
	  return true;

  m_initialized = m_DllBcmHost->Load();
  if(!m_initialized)
    return false;

  log_info("bcm_host_init");
  m_DllBcmHost->bcm_host_init();
  m_refcount++;

  return true;
}

void CRBP::Deinitialize()
{
  QMutexLocker l(&m_mutex);
  m_refcount--;
  if (m_refcount > 0)
	  return;

  log_info("bcm_host_deinit");
  m_DllBcmHost->bcm_host_deinit();

  if(m_initialized)
    m_DllBcmHost->Unload();

  m_initialized = false;
}
