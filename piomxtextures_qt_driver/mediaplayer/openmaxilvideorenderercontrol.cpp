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

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QQuickItem>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QTimer>
#include <QMutex>

#include "openmaxilplayercontrol.h"

#include "omx_logging.h"

/*------------------------------------------------------------------------------
|    OpenMAXILVideoBuffer class
+-----------------------------------------------------------------------------*/
class OpenMAXILVideoBuffer : public QAbstractVideoBuffer
{
public:
   OpenMAXILVideoBuffer(HandleType handleType, GLuint textureId) :
      QAbstractVideoBuffer(handleType)
   {
      m_handleType = handleType;
      m_textureId  = textureId;
   }

   virtual ~OpenMAXILVideoBuffer() {
      log_dtor_func;
   }

   QVariant handle() const {
      return QVariant::fromValue<unsigned int>(m_textureId);
   }

   void setHandle(GLuint textureId) {
      m_textureId = textureId;
   }

   uchar* map(MapMode mode, int* numBytes, int* bytesPerLine) {
      return NULL;
   }

   MapMode mapMode() const {
      return ReadOnly;
   }

   void unmap() {}

private:
   HandleType m_handleType;
   uint m_textureId;
};

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::OpenMAXILVideoRendererControl
+-----------------------------------------------------------------------------*/
OpenMAXILVideoRendererControl::OpenMAXILVideoRendererControl(OpenMAXILPlayerControl* control, QObject* parent) :
   QVideoRendererControl(parent),
   m_control(control),
   m_mediaProcessor(control->getMediaProcessor()),
   m_mutex(QMutex::Recursive),
   m_surface(NULL),
   m_surfaceFormat(NULL),
   m_frame(NULL),
   m_buffer(NULL)
{
   //connect(m_mediaProcessor, SIGNAL(stateChanged(OMX_MediaProcessor::OMX_MediaProcessorState)),
   //        this, SLOT(onMediaPlayerStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState)));

   OMX_EGLBufferProviderSh provider = m_mediaProcessor->m_provider;
   connect(provider.get(), SIGNAL(texturesReady()),
           this, SLOT(onTexturesReady()));
   connect(provider.get(), SIGNAL(texturesFreed()),
           this, SLOT(onTexturesFreed()));
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::~OpenMAXILVideoRendererControl
+-----------------------------------------------------------------------------*/
OpenMAXILVideoRendererControl::~OpenMAXILVideoRendererControl()
{
   QMutexLocker locker(&m_mutex);
   log_dtor_func;

   // Stop the surface first.
   if (m_surface)
      m_surface->stop();

   log_verbose("Freeing frame and format...");
   delete m_surfaceFormat;
   delete m_frame;
#if 0
   // NOTE: Do not free the buffer. It is already freed (I guess by
   // deleting the frame somehow).
   delete m_buffer;
#endif

   m_surfaceFormat = NULL;
   m_frame         = NULL;
   m_buffer        = NULL;
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::setSurface
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::setSurface(QAbstractVideoSurface* surface)
{
   QMutexLocker locker(&m_mutex);
   log_debug_func;

   if (m_surface && m_surface->isActive())
      m_surface->stop();
   m_surface = surface;
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::surface
+-----------------------------------------------------------------------------*/
QAbstractVideoSurface* OpenMAXILVideoRendererControl::surface() const
{
   log_debug_func;
   return m_surface;
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::onTextureReady
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::onTexturesReady()
{
   QMutexLocker locker(&m_mutex);
   onTexturesFreed(); // Cleanup.

   log_verbose("Setting up for new textures...");
   OMX_EGLBufferProviderSh provider = m_mediaProcessor->m_provider;
   if (!provider.get()) {
      log_warn("Invalid provider.");
      return;
   }

   QList<OMX_TextureData*> textures = provider->getBuffers();
   if (textures.size() <= 0) {
      log_warn("No textures in the provider.");
      return;
   }

   OMX_TextureData* texture = textures.at(0);
   if (!texture) {
      log_warn("Couldn't get a valid texture.");
      return;
   }

   // Just take one random texture to determine the size. Do not place
   // any texture yet in the scene as I can't guarantee it is filled
   // already with valid data.
   m_buffer = new OpenMAXILVideoBuffer(
            QAbstractVideoBuffer::GLTextureHandle,
            0
            );
   m_frame = new QVideoFrame(
            m_buffer,
            texture->m_textureSize,
            QVideoFrame::Format_RGB565
            );
   m_surfaceFormat = new QVideoSurfaceFormat(
            texture->m_textureSize,
            QVideoFrame::Format_RGB565,
            QAbstractVideoBuffer::GLTextureHandle
            );
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::onTextureInvalidated
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::onTexturesFreed()
{
   QMutexLocker locker(&m_mutex);

   if (m_surfaceFormat) {
      log_verbose("Destroying QVideoSurfaceFormat...");
      delete m_surfaceFormat;
      m_surfaceFormat = NULL;
   }

   if (m_frame) {
      log_verbose("Destroying QVideoFrame...");
      delete m_frame;
      m_frame = NULL;
   }
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::onUpdateTriggered
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::onUpdateTriggered()
{
   QMutexLocker locker(&m_mutex);

   if (UNLIKELY(!m_mediaProcessor || !m_mediaProcessor->m_provider.get()))
      return;
   if (UNLIKELY(!m_surface || !m_frame || !m_surfaceFormat))
      return;

   if (UNLIKELY(!m_surface->isActive() && !m_surface->start(*m_surfaceFormat)))
      log_warn("Failed to start surface.");

   GLuint t = m_mediaProcessor->m_provider->getNextTexture();

   m_buffer->setHandle(t);
   m_surface->present(*m_frame);

   //assert(m_control);
   //m_control->requestUpdate();
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::onMediaPlayerStateChanged
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::onMediaPlayerStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState state)
{
   // FIXME: I should stop the update requests when there is no playback.
#if 0
   if (state == OMX_MediaProcessor::STATE_PLAYING)
      m_updateTimer->start();
   else
      m_updateTimer->stop();
#endif
}
