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
#include <QOpenGLContext>
#include <QThread>

#include <IL/OMX_Video.h>
#include <GLES2/gl2.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <assert.h>
#include <memory>

#include "omx_globals.h"
#include "omx_logging.h"
#include "omx_staticconf.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
class QQuickItem;

#define TEXTURE_COUNT (OMX_StaticConf::getTextureCount())
//#define DEBUG_TEXTURE_PROVIDER

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
|    OMX_EGLBufferProvider class
+-----------------------------------------------------------------------------*/
class OMX_EGLBufferProvider : public QObject
{
	Q_OBJECT
public:
   OMX_EGLBufferProvider() :
      QObject()
	 , m_semEmpty(TEXTURE_COUNT)
	 , m_currentDecoder(NULL)
	 , m_currentRenderer(NULL)
    , m_mutex(QMutex::Recursive)
    , m_initialized(false)
    , m_oglContext(NULL)
    , m_eglContext(NULL)
    , m_eglSurface(NULL)
    , m_eglDisplay(NULL)
   {}

	~OMX_EGLBufferProvider();

   bool init();
   bool instantiateTextures(const QSize& s);
   OMX_TextureData* instantiateTexture(const QSize& size);
	bool textureInstantiated();

   OMX_TextureData* getNextEmptyBuffer();
	GLuint getNextTexture();

	bool registerFilledBuffer(OMX_BUFFERHEADERTYPE* buffer);

   QList<OMX_TextureData*> getBuffers() {
		QMutexLocker locker(&m_mutex);
      return m_available;
   }

   int getBufferCount() {
		QMutexLocker locker(&m_mutex);
      return m_available.size();
   }

	void flush() {
		QMutexLocker locker(&m_mutex);

#if 0
		foreach (OMX_TextureData* data, m_filledQueue)
			m_emptyQueue.push_back(data);
		if (m_currentDecoder)
			m_emptyQueue.push_back(m_currentDecoder);
		if (m_currentRenderer)
			m_emptyQueue.push_back(m_currentRenderer);

		m_filledQueue.clear();
		m_currentDecoder = NULL;

		assert(m_emptyQueue.size() == m_available.size());
#endif

		m_emptyQueue.clear();
		m_filledQueue.clear();
		m_currentDecoder = NULL;
		m_currentRenderer = NULL;

		m_emptyQueue.append(m_available);
		m_semEmpty.release(TEXTURE_COUNT - m_semEmpty.available());
	}

public slots:
	void free();
   void deinit();

signals:
	void texturesReady();
	void texturesFreed();
	void frameReady();

private:
   QQueue<OMX_TextureData*> m_filledQueue;
   QQueue<OMX_TextureData*> m_emptyQueue;
   QSemaphore m_semEmpty;
   QList<OMX_TextureData*> m_available;
	OMX_TextureData* m_currentDecoder;
	OMX_TextureData* m_currentRenderer;
   QMutex m_mutex;
   bool m_initialized;

   // The context to be used to create buffers and textures.
   QOpenGLContext* m_oglContext;
   EGLContext m_eglContext;
   EGLSurface m_eglSurface;
   EGLDisplay m_eglDisplay;
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
      log_err("Leaking data in GPU: m_filledQueue size is %d.", m_filledQueue.size());
	if (m_emptyQueue.size() > 0)
      log_err("Leaking data in GPU: m_emptyQueue size is %d.", m_emptyQueue.size());

   if (m_initialized)
      log_err("Provider is being destroyed while still inizialized. Leaking GPU resources.");
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::textureInstantiated
+-----------------------------------------------------------------------------*/
inline
bool OMX_EGLBufferProvider::textureInstantiated()
{
   return m_available.size() != 0;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::init
+-----------------------------------------------------------------------------*/
inline
bool OMX_EGLBufferProvider::init()
{
   // This method MUST be called in the same thread that is then creating buffers and
   // textures.

   QMutexLocker locker(&m_mutex);
   if (m_initialized)
      return true;
   if (QOpenGLContext::currentContext()) {
      log_info("Using old sync OGL architecture.");
      m_oglContext = QOpenGLContext::currentContext();
      m_eglContext = eglGetCurrentContext();

      return true;
   }

   log_info("Initializing buffer provider...");

   QOpenGLContext* sharedOglContext = QOpenGLContext::globalShareContext();
   if (!sharedOglContext)
      return log_err("Failed to get shared OGL context. Please enable it with the proper attr.");

   log_verbose("Creating a new OpenGL context in thread %p.", QThread::currentThreadId());
   m_oglContext = new QOpenGLContext();
   m_oglContext->setShareContext(sharedOglContext);
   m_oglContext->makeCurrent(sharedOglContext->surface());

   // Get global strictures.
   m_eglDisplay = get_egl_display();
   if (!m_eglDisplay)
      return false;

   EGLContext eglGlobalContext = get_global_egl_context();
   if (!eglGlobalContext)
      return false;

   // Get global EGL context config.
   const int MAX_CONFIG = 100;
   EGLConfig configs[MAX_CONFIG];
   int confCount;
   EGLBoolean success = eglGetConfigs(m_eglDisplay, configs, 100, &confCount);
   if (success != EGL_TRUE)
      return log_err("Failed to get EGL configs: %u.", success);

   EGLint eglVal;
   success = eglQueryContext(m_eglDisplay, eglGlobalContext, EGL_CONFIG_ID, &eglVal);
   if (success != EGL_TRUE)
      return log_err("Failed to get EGL context config: %u.", success);

   log_debug("Global EGL context config index is %d.", eglVal);
   if (eglVal >= MAX_CONFIG)
      return log_err("Insufficient array size for EGL configs.");

   EGLConfig eglConfig = configs[eglVal];
   if (!eglConfig)
      return log_err("Failed to get EGL config.");

   // Create new EGL context.
   m_eglContext = eglCreateContext(m_eglDisplay, eglConfig, eglGlobalContext, NULL);
   if (!m_eglContext)
      return log_err("Failed to create new EGL context.");

   m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, eglConfig, NULL);
   if (eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext) != EGL_TRUE)
      return log_err("Failed to make current EGL ctx.");

   return (m_initialized = true);
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::instantiateBuffers
+-----------------------------------------------------------------------------*/
inline
bool OMX_EGLBufferProvider::instantiateTextures(const QSize& s)
{
	QMutexLocker locker(&m_mutex);
	if (!m_available.isEmpty()) {
		if (m_available.at(0)->m_textureSize == s) {
			flush();
			return log_verbose("Reusing allocated textures.");
		}
	}

	// Free all textures.
	free();

#if 0
	if (UNLIKELY(m_texCount != 0)) {
		log_warn("There are currently %d textures available already.", m_texCount);
		return false;
	}

	if (UNLIKELY(count <= 0)) {
		log_warn("It makes no sense to instantiate <= 0 textures.");
		return false;
	}
#endif

   m_semEmpty.release(TEXTURE_COUNT - m_semEmpty.available());
   for (int i = 0; i < TEXTURE_COUNT; i++) {
      log_info("Instantiating texture data...");
      OMX_TextureData* tex = instantiateTexture(s);
      m_emptyQueue.append(tex);
      m_available.append(tex);
	}

	emit texturesReady();

   return true;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::instantiateTexture
+-----------------------------------------------------------------------------*/
inline OMX_TextureData* OMX_EGLBufferProvider::instantiateTexture(const QSize& size)
{
   EGLint attr[] = {EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE};

   GLuint textureId;
   glGenTextures(1, &textureId);
   glBindTexture(GL_TEXTURE_2D, textureId);

   // It seems that only 4byte pixels is supported here.
   //GLubyte* pixel = new GLubyte[size.width()*size.height()*2];
   //memset(pixel, 0, size.width()*size.height()*2);
   GLubyte* pixel = NULL;
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.width(), size.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, pixel);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   log_info("Creating EGLImageKHR...");
   EGLImageKHR eglImage = eglCreateImageKHR(
            m_eglDisplay,
            m_eglContext,
            EGL_GL_TEXTURE_2D_KHR,
            (EGLClientBuffer)textureId,
            attr
            );
   log_verbose("EGL image %d created...", eglImage);

   EGLint eglErr = eglGetError();
   if (eglErr != EGL_SUCCESS) {
      LOG_ERROR(LOG_TAG, "Failed to create KHR image: %d.", eglErr);
      return 0;
   }

   log_verbose("Creating OMX_TextureData...");
   OMX_TextureData* textureData = new OMX_TextureData;
   textureData->m_textureId   = textureId;
   textureData->m_textureData = pixel;
   textureData->m_eglImage    = eglImage;
   textureData->m_textureSize = size;

   return textureData;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::getNextTexture
+-----------------------------------------------------------------------------*/
inline
GLuint OMX_EGLBufferProvider::getNextTexture()
{
	QMutexLocker locker(&m_mutex);

	OMX_TextureData* data;
	if (LIKELY(!m_filledQueue.empty()))
		data = m_filledQueue.dequeue();
	else {
		if (LIKELY(m_currentRenderer != NULL)) {
			log_disabled("No decoded frame to show. Showing the same twice.");
			return m_currentRenderer->m_textureId;
		}

      log_warn("No decoded frame to show at all. Let's render black.");
		return 0;
	}

   if (LIKELY(m_currentRenderer != NULL)) {
		m_emptyQueue.enqueue(m_currentRenderer);

      m_semEmpty.release();
#ifdef DEBUG_TEXTURE_PROVIDER
      log_debug("sem released: %d.", m_semEmpty.available());
#endif
   }

	m_currentRenderer = data;

	return data->m_textureId;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::registerFilledBuffer
+-----------------------------------------------------------------------------*/
inline
bool OMX_EGLBufferProvider::registerFilledBuffer(OMX_BUFFERHEADERTYPE* buffer)
{
	QMutexLocker locker(&m_mutex);
	if (UNLIKELY(m_currentDecoder == NULL))
		return true;

	m_filledQueue.enqueue(m_currentDecoder);
	m_currentDecoder = NULL;

   emit frameReady();

	return true;
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::getNextEmptyBuffer
+-----------------------------------------------------------------------------*/
inline
OMX_TextureData* OMX_EGLBufferProvider::getNextEmptyBuffer()
{
#ifdef DEBUG_TEXTURE_PROVIDER
   log_debug("Waiting for the sem: %d.", m_semEmpty.available());
#endif
   const bool found = m_semEmpty.tryAcquire(1, 40);
#ifdef DEBUG_TEXTURE_PROVIDER
   log_debug("Acquired sem: %d.", m_semEmpty.available());
#endif

	QMutexLocker locker(&m_mutex);

   if (LIKELY(found))
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

   m_semEmpty.release(TEXTURE_COUNT - m_semEmpty.available());

   emit texturesFreed();
}

/*------------------------------------------------------------------------------
|    OMX_EGLBufferProvider::deinit
+-----------------------------------------------------------------------------*/
inline void OMX_EGLBufferProvider::deinit()
{
   QMutexLocker locker(&m_mutex);
   if (!m_initialized)
      return;

   // Free EGL surface.
   log_verbose("Destroying EGL surface...");
   EGLBoolean ret = eglDestroySurface(m_eglDisplay, m_eglSurface);
   if (ret != EGL_TRUE)
      log_warn("Failed to destroy EGL surface: %u.", ret);
   m_eglSurface = NULL;

   // Free EGL context.
   log_verbose("Destroying EGL aux context...");
   ret = eglDestroyContext(m_eglDisplay, m_eglContext);
   if (ret != EGL_TRUE)
      log_warn("Failed to destroy EGL aux context: %u.", ret);

   // Destroy OGL context.
   log_verbose("Destroying OGL aux context...");
   delete m_oglContext;

   m_initialized = false;
}

#endif // OMX_TEXTUREPROVIDERQQUICKITEM_H
