/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    07.20.2013
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

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <Qt>

#include <cmath>

#include "omx_playeraudio.h"
#include "lc_logging.h"

using namespace std;


/*------------------------------------------------------------------------------
|    OMX_PlayerAudio::OMX_PlayerAudio
+-----------------------------------------------------------------------------*/
OMX_PlayerAudio::OMX_PlayerAudio() : OMXPlayerAudio()
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    OMX_PlayerAudio::~OMX_PlayerAudio
+-----------------------------------------------------------------------------*/
OMX_PlayerAudio::~OMX_PlayerAudio()
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    OMX_PlayerAudio::SetMuted
+-----------------------------------------------------------------------------*/
void OMX_PlayerAudio::SetMuted(bool mute)
{
       m_mute = mute;

       if (m_decoder)
	       m_decoder->SetMute(m_mute);
}

/*------------------------------------------------------------------------------
|    OMX_PlayerAudio::GetMuted
+-----------------------------------------------------------------------------*/
bool OMX_PlayerAudio::GetMuted()
{
       return m_mute;
}

/*------------------------------------------------------------------------------
|    OMX_PlayerAudio::SetCurrentVolume
+-----------------------------------------------------------------------------*/
/**
 * @brief OMX_PlayerAudio::SetCurrentVolume
 * @param volume Volume in mB or percentage [0, 100].
 * @param linear
 */
void OMX_PlayerAudio::SetCurrentVolume(long volume, bool linear)
{
   if (!linear) {
      OMXPlayerAudio::SetVolume(volume);
      return;
   }

   // I supposed it was possible to get the available range from OpenMAX
   // but it seems to always return 0.
#ifdef ENABLE_RANGE_FROM_OMX
   // Get current configuration to determine max and min mB.
   OMX_AUDIO_CONFIG_VOLUMETYPE omxVolume;
   OMX_INIT_STRUCTURE(omxVolume);
   omxVolume.nPortIndex = m_decoder->m_omx_render.GetInputPort();
   omxVolume.bLinear = OMX_FALSE;
   OMX_ERRORTYPE omx_err = m_decoder->m_omx_render.GetConfig(OMX_IndexConfigAudioVolume, &omxVolume);
   if (omx_err != OMX_ErrorNone) {
      LOG_WARNING(LOG_TAG, "%s - error getting OMX_IndexConfigAudioVolume, error 0x%08x\n",
                  Q_FUNC_INFO, omx_err);
      return;
   }

   // Determine the mB value to be used.
   OMX_S32 mbMax = omxVolume.sVolume.nMax;
   OMX_S32 mbMin = omxVolume.sVolume.nMin;
#else
   OMX_S32 mbMax = 2;
   OMX_S32 mbMin = -6;
#endif

   double expMin   = exp(mbMin);
   double expMax   = exp(mbMax);
   double mbVolume = log((double)volume/100*(expMax - expMin) + expMin);
   LOG_VERBOSE(LOG_TAG, "Setting volume to %fmB.", mbVolume);

   // omxplayer expects millibels here.
   OMXPlayerAudio::SetVolume(mbVolume*1000);
}

/*------------------------------------------------------------------------------
|    OMX_PlayerAudio::GetCurrentVolume
+-----------------------------------------------------------------------------*/
long OMX_PlayerAudio::GetCurrentVolume(bool linear)
{
   return 0;

#if 0
   long mbVol = OMXPlayerAudio::GetCurrentVolume();
   if (!linear)
      return mbVol;

   OMX_S32 mbMax = 2;
   OMX_S32 mbMin = -6;
   double expMin = exp(mbMin);
   double expMax = exp(mbMax);
   double num = 100*exp(mbVol/1000) - expMin;
   double den = expMax - expMin;

   LOG_VERBOSE(LOG_TAG, "Volume: %ld.", (long)(num/den));
   return (long)(num/den);
#endif
}
