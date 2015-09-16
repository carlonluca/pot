/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    04.14.2013
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
 * along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPENMAXILVIDEORENDERERCONTROL_H
#define OPENMAXILVIDEORENDERERCONTROL_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include "QtMultimedia/qvideorenderercontrol.h"
#include "QtMultimedia/qvideoframe.h"
#include "QtMultimedia/qvideosurfaceformat.h"
#include "QtMultimedia/qmediaplayer.h"

#include <omx_textureprovider.h>
#include <omx_mediaprocessor.h>

/*------------------------------------------------------------------------------
|    defintions
+-----------------------------------------------------------------------------*/
class OpenMAXILVideoBuffer;
class OpenMAXILPlayerControl;

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl class
+-----------------------------------------------------------------------------*/
class OpenMAXILVideoRendererControl : public QVideoRendererControl
{
   Q_OBJECT
public:
   explicit OpenMAXILVideoRendererControl(OpenMAXILPlayerControl* control, QObject* parent = 0);
   ~OpenMAXILVideoRendererControl();
   void setSurface(QAbstractVideoSurface* surface);
   QAbstractVideoSurface* surface() const;

public slots:
   void onTexturesReady();
   void onTexturesFreed();
   void onUpdateTriggered();
   void onMediaPlayerStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState);

private:
   OpenMAXILPlayerControl* m_control;
   OMX_MediaProcessor* m_mediaProcessor;
   QMutex m_mutex;

   QAbstractVideoSurface* m_surface;
   QVideoSurfaceFormat* m_surfaceFormat;
   QVideoFrame* m_frame;
   OpenMAXILVideoBuffer* m_buffer;
};

#endif // OPENMAXILVIDEORENDERERCONTROL_H
