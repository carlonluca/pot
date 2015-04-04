/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.19.2012
 *
 * Copyright (c) 2012 Luca Carlon. All rights reserved.
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

#ifndef OMX_TEXTUREPROVIDERQQUICKITEM_H
#define OMX_TEXTUREPROVIDERQQUICKITEM_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QObject>
#include <QSize>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include <IL/OMX_Video.h>
#include <GLES2/gl2.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <assert.h>
#include <memory>

#include "lc_logging.h"
#include "omx_globals.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
class QQuickItem;


/*------------------------------------------------------------------------------
|    OMX_TextureData class
+-----------------------------------------------------------------------------*/
class OMX_TextureData
{
public:
   OMX_TextureData();
   OMX_TextureData(const OMX_TextureData& textureData);

   ~OMX_TextureData();

   void freeData();

   GLuint      m_textureId;
   GLubyte*    m_textureData;
   EGLImageKHR m_eglImage;
   QSize       m_textureSize;
   OMX_BUFFERHEADERTYPE* m_omxBuffer;
};

/*------------------------------------------------------------------------------
|    OMX_TextureProvider class
+-----------------------------------------------------------------------------*/
/**
 * @brief The OMX_TextureProviderQQuickItem class can be instantiate by
 * a QQuickItem and passed to a OMX element. It MUST be instantiated in
 * the renderer thread.
 */
class OMX_TextureProvider
{
public:
   virtual ~OMX_TextureProvider() {}
   virtual OMX_TextureData* instantiateTexture(QSize size) = 0;
   virtual void freeTexture(OMX_TextureData* textureData) = 0;
};
typedef std::shared_ptr<OMX_TextureProvider> OMX_TextureProviderSh;

/*------------------------------------------------------------------------------
|    OMX_TextureProviderQQuickItem class
+-----------------------------------------------------------------------------*/
class OMX_TextureProviderQQuickItem : public OMX_TextureProvider
{
public:
   OMX_TextureProviderQQuickItem() : OMX_TextureProvider() {
      // Do nothing.
   }

   virtual ~OMX_TextureProviderQQuickItem() {}

   OMX_TextureData* instantiateTexture(QSize size);
   void freeTexture(OMX_TextureData* textureData);
};

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider class
+-----------------------------------------------------------------------------*/
class OMX_EGLBufferProvider : public QObject
{
   Q_OBJECT
public:
   OMX_EGLBufferProvider() :
      m_texProvider(new OMX_TextureProviderQQuickItem)
    , m_current(NULL)
    , m_currentRendered(NULL)
   {}

   ~OMX_EGLBufferProvider() {
      m_filledMx.lock();
      if (m_filledQueue.size() > 0)
         log_warn("Leaking data in GPU: m_filledQueue size is %d.", m_filledQueue.size());
      m_filledMx.unlock();

      m_emptyMx.lock();
      if (m_emptyQueue.size() > 0)
         log_warn("Leaking data in GPU: m_emptyQueue size is %d.", m_emptyQueue.size());
      m_emptyMx.unlock();
   }

   bool instantiateTextures(const QSize& s, int count);
   bool textureInstantiated() {
      return !(m_filledQueue.isEmpty() && m_emptyQueue.isEmpty() && !m_current);
   }

   OMX_TextureData* getNextEmptyBuffer();
   GLuint getNextTexture() {
      QMutexLocker locker1(&m_filledMx);
      QMutexLocker locker2(&m_emptyMx);

      OMX_TextureData* data = getNextFilledBuffer();
      if (!data)
         return m_currentRendered ? m_currentRendered->m_textureId : 0;

      if (m_currentRendered)
         appendEmptyBuffer(m_currentRendered);
      m_currentRendered = data;

      return data->m_textureId;
   }

   bool registerFilledBuffer(OMX_BUFFERHEADERTYPE* buffer) {
      {
         QMutexLocker locker1(&m_filledMx);
         QMutexLocker locker2(&m_emptyMx);

         if (!m_current)
            return false;
         m_current->m_omxBuffer = buffer;
      }

      appendFilledBuffer(m_current);
      m_current = NULL;

      return true;
   }

   void appendFilledBuffer(OMX_TextureData* buffer);

   void cleanTextures() {
      QMutexLocker locker1(&m_filledMx);
      QMutexLocker locker2(&m_emptyMx);

      m_current = NULL;
      m_emptyQueue.clear();
      m_filledQueue.clear();

      foreach (OMX_TextureData* data, m_available)
         m_emptyQueue << data;
   }

   QList<OMX_TextureData*> getBuffers() {
      QMutexLocker locker1(&m_filledMx);
      QMutexLocker locker2(&m_emptyMx);

      return m_available;
   }

   int getBufferCount() {
      QMutexLocker locker1(&m_filledMx);
      QMutexLocker locker2(&m_emptyMx);

      return m_available.size();
   }

   void free() {
      QMutexLocker locker1(&m_filledMx);
      QMutexLocker locker2(&m_emptyMx);

      foreach (OMX_TextureData* data, m_available)
         data->freeData();

      m_current = NULL;
      m_currentRendered = NULL;

      m_filledQueue.clear();
      m_emptyQueue.clear();
      m_available.clear();

      emit texturesFreed();
   }

signals:
   void texturesReady();
   void texturesFreed();

private:
   OMX_TextureData* getNextFilledBuffer();
   void appendEmptyBuffer(OMX_TextureData* buffer);

   OMX_TextureProviderSh m_texProvider;
   QQueue<OMX_TextureData*> m_filledQueue;
   QQueue<OMX_TextureData*> m_emptyQueue;
   QList<OMX_TextureData*> m_available;
   OMX_TextureData* m_current;
   OMX_TextureData* m_currentRendered;
   QMutex m_filledMx;
   QMutex m_emptyMx;
};
typedef std::shared_ptr<OMX_EGLBufferProvider> OMX_EGLBufferProviderSh;

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::instantiateBuffers
+-----------------------------------------------------------------------------*/
inline
bool OMX_EGLBufferProvider::instantiateTextures(const QSize& s, int count)
{
   QMutexLocker locker(&m_emptyMx);
   for (int i = 0; i < count; i++) {
      log_info("Instantiating texture data...");
      OMX_TextureData* tex = m_texProvider->instantiateTexture(s);
      m_emptyQueue.append(tex);
      m_available.append(tex);
   }

   emit texturesReady();

   return true;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::getNextFilledBuffer
+-----------------------------------------------------------------------------*/
inline
OMX_TextureData* OMX_EGLBufferProvider::getNextFilledBuffer()
{
   if (!m_filledQueue.empty())
      return m_filledQueue.dequeue();

   return NULL;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::getNextEmptyBuffer
+-----------------------------------------------------------------------------*/
inline
OMX_TextureData* OMX_EGLBufferProvider::getNextEmptyBuffer()
{
   {
      QMutexLocker locker(&m_emptyMx);
      if (!m_emptyQueue.empty())
         return (m_current = m_emptyQueue.dequeue());
   }

   {
      QMutexLocker locker(&m_filledMx);
      if (!m_filledQueue.empty()) {
         log_warn("One frame couldn't be shown.");
         return (m_current = m_filledQueue.dequeue());
      }
   }

   // We should never come to this.
   log_warn("No buffer available in any queue.");
   return NULL;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::appendFilledBuffer
+-----------------------------------------------------------------------------*/
inline
void OMX_EGLBufferProvider::appendFilledBuffer(OMX_TextureData* buffer)
{
   QMutexLocker locker(&m_filledMx);
   m_filledQueue.enqueue(buffer);
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::appendEmptyBuffer
+-----------------------------------------------------------------------------*/
inline
void OMX_EGLBufferProvider::appendEmptyBuffer(OMX_TextureData* buffer)
{
   m_emptyQueue.enqueue(buffer);
}

#endif // OMX_TEXTUREPROVIDERQQUICKITEM_H
