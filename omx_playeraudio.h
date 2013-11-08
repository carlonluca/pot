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

#ifndef OMX_AUDIO_H
#define OMX_AUDIO_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include "OMXPlayerAudio.h"


/*------------------------------------------------------------------------------
|    OMX_PlayerAudio class
+-----------------------------------------------------------------------------*/
class OMX_PlayerAudio : public OMXPlayerAudio
{
public:
   OMX_PlayerAudio();
   virtual ~OMX_PlayerAudio();

   void SetCurrentVolume(long volume, bool linear);
   long GetCurrentVolume(bool linear);

   void SetMuted(bool mute);
   bool GetMuted();
};

#endif // OMX_AUDIO_H
