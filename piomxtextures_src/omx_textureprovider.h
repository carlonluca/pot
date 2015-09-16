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

#include "omx_globals.h"
#include "omx_utils.h"
#include "omx_logging.h"

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
		QObject()
	 , m_texProvider(new OMX_TextureProviderQQuickItem)
	 , m_currentDecoder(NULL)
	 , m_currentRenderer(NULL)
	 , m_texCount(0)
   {}

	~OMX_EGLBufferProvider();

   bool instantiateTextures(const QSize& s, int count);
	bool textureInstantiated();

   OMX_TextureData* getNextEmptyBuffer();
	GLuint getNextTexture();

	bool registerFilledBuffer(OMX_BUFFERHEADERTYPE* buffer);

	void cleanTextures();

   QList<OMX_TextureData*> getBuffers() {
		QMutexLocker locker1(&m_mutex);
      return m_available;
   }

   int getBufferCount() {
		QMutexLocker locker1(&m_mutex);
      return m_available.size();
   }

	void free();

signals:
	void texturesReady();
	void texturesFreed();

private:
   OMX_TextureProviderSh m_texProvider;
   QQueue<OMX_TextureData*> m_filledQueue;
   QQueue<OMX_TextureData*> m_emptyQueue;
   QList<OMX_TextureData*> m_available;
	OMX_TextureData* m_currentDecoder;
	OMX_TextureData* m_currentRenderer;
	QMutex m_mutex;
	int m_texCount;
};

typedef std::shared_ptr<OMX_EGLBufferProvider> OMX_EGLBufferProviderSh;

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::~OMX_EGLBufferProvider
+-----------------------------------------------------------------------------*/
inline OMX_EGLBufferProvider::~OMX_EGLBufferProvider()
{
	log_dtor_func;

	QMutexLocker locker(&m_mutex);
	if (m_filledQueue.size() > 0)
		log_warn("Leaking data in GPU: m_filledQueue size is %d.", m_filledQueue.size());
	if (m_emptyQueue.size() > 0)
		log_warn("Leaking data in GPU: m_emptyQueue size is %d.", m_emptyQueue.size());
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::textureInstantiated
+-----------------------------------------------------------------------------*/
inline
bool OMX_EGLBufferProvider::textureInstantiated()
{
	return m_texCount != 0;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::instantiateBuffers
+-----------------------------------------------------------------------------*/
inline
bool OMX_EGLBufferProvider::instantiateTextures(const QSize& s, int count)
{
	QMutexLocker locker(&m_mutex);
	if (UNLIKELY(m_texCount != 0)) {
		log_warn("There are currently %d textures available already.", m_texCount);
		return false;
	}

	if (UNLIKELY(count <= 0)) {
		log_warn("It makes no sense to instantiate <= 0 textures.");
		return false;
	}

   for (int i = 0; i < count; i++) {
      log_info("Instantiating texture data...");
      OMX_TextureData* tex = m_texProvider->instantiateTexture(s);
      m_emptyQueue.append(tex);
      m_available.append(tex);
	}

	m_texCount = count;

	emit texturesReady();

   return true;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::getNextTexture
+-----------------------------------------------------------------------------*/
inline
GLuint OMX_EGLBufferProvider::getNextTexture()
{
	QMutexLocker locker1(&m_mutex);

	OMX_TextureData* data;
	if (LIKELY(!m_filledQueue.empty()))
		data = m_filledQueue.dequeue();
	else {
		if (LIKELY(m_currentRenderer != NULL)) {
			log_warn("No decoded frame to show. Showing the same twice.");
			return m_currentRenderer->m_textureId;
		}

		log_warn("No decoded frame to show at all. Let' render black.");
		return 0;
	}

	if (LIKELY(m_currentRenderer != NULL))
		m_emptyQueue.enqueue(m_currentRenderer);
	m_currentRenderer = data;

	return data->m_textureId;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::registerFilledBuffer
+-----------------------------------------------------------------------------*/
inline
bool OMX_EGLBufferProvider::registerFilledBuffer(OMX_BUFFERHEADERTYPE* buffer)
{
	QMutexLocker locker1(&m_mutex);

	if (UNLIKELY(m_currentDecoder == NULL)) {
		log_warn("Can't register a filled buffer. None is being filled.");
		return false;
	}

	m_filledQueue.enqueue(m_currentDecoder);
	m_currentDecoder = NULL;

	return true;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::getNextEmptyBuffer
+-----------------------------------------------------------------------------*/
inline
OMX_TextureData* OMX_EGLBufferProvider::getNextEmptyBuffer()
{
	QMutexLocker locker(&m_mutex);

	if (LIKELY(!m_emptyQueue.empty()))
		return (m_currentDecoder = m_emptyQueue.dequeue());
	if (LIKELY(!m_filledQueue.empty())) {
		log_warn("One frame couldn't be shown.");
		return (m_currentDecoder = m_filledQueue.dequeue());
	}

   // We should never come to this.
   log_warn("No buffer available in any queue.");
   return NULL;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::free
+-----------------------------------------------------------------------------*/
inline void OMX_EGLBufferProvider::free()
{
	QMutexLocker locker(&m_mutex);

	foreach (OMX_TextureData* data, m_available)
		data->freeData();

	m_currentDecoder = NULL;
	m_currentRenderer = NULL;

	m_filledQueue.clear();
	m_emptyQueue.clear();
	m_available.clear();

	m_texCount = 0;

	emit texturesFreed();
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::cleanTextures
+-----------------------------------------------------------------------------*/
inline void OMX_EGLBufferProvider::cleanTextures()
{
	QMutexLocker locker(&m_mutex);

	m_currentDecoder = NULL;
	m_emptyQueue.clear();
	m_filledQueue.clear();

	foreach (OMX_TextureData* data, m_available)
		m_emptyQueue << data;
}

#endif // OMX_TEXTUREPROVIDERQQUICKITEM_H
