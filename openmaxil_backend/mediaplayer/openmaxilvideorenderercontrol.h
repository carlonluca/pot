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

#include <omx_textureprovider.h>

/*------------------------------------------------------------------------------
|    defintions
+-----------------------------------------------------------------------------*/
class OpenMAXILVideoBuffer;

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl class
+-----------------------------------------------------------------------------*/
class OpenMAXILVideoRendererControl : public QVideoRendererControl
{
   Q_OBJECT
public:
   explicit OpenMAXILVideoRendererControl(QObject* parent = 0);
   ~OpenMAXILVideoRendererControl();
   void setSurface(QAbstractVideoSurface* surface);
   QAbstractVideoSurface* surface() const;

public slots:
   void onTextureReady(const OMX_TextureData* textureData);
   void onTextureInvalidated();
   void onUpdateTriggered();

private:
   QAbstractVideoSurface* m_surface;
   QVideoSurfaceFormat* m_surfaceFormat;
   QVideoFrame* m_frame;
   OpenMAXILVideoBuffer* m_buffer;
   uint m_textureId;
   QTimer* m_updateTimer;
};

#endif // OPENMAXILVIDEORENDERERCONTROL_H
