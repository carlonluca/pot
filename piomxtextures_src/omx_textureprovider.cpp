/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    11.01.2012
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
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QGuiApplication>

// Private headers.
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <IL/OMX_Broadcom.h>

#include "omx_textureprovider.h"
#include "omx_globals.h"
#include "omx_logging.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#ifdef DEBUG_TDATA_LEAKS
int OMX_TextureData::count = 0;
#endif // DEBUG_TDATA_LEAKS

/*------------------------------------------------------------------------------
|    check_gl_error
+-----------------------------------------------------------------------------*/
void check_gl_error() {
#ifdef ENABLE_LOG_DEBUG
   GLenum err = glGetError();

   while (err != GL_NO_ERROR) {
      const char* error;

      switch (err) {
      case GL_INVALID_OPERATION:
         error = "INVALID_OPERATION";
         break;
      case GL_INVALID_ENUM:
         error = "INVALID_ENUM";
         break;
      case GL_INVALID_VALUE:
         error = "INVALID_VALUE";
         break;
      case GL_OUT_OF_MEMORY:
         error = "OUT_OF_MEMORY";
         break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
         error = "INVALID_FRAMEBUFFER_OPERATION";
         break;
      default:
         return;
      }

		log_err("GL error: %s.", error);

      err = glGetError();
   }
#else
   return;
#endif
}

/*------------------------------------------------------------------------------
|    OMX_TextureData::OMX_TextureData
+-----------------------------------------------------------------------------*/
OMX_TextureData::OMX_TextureData() :
   m_textureId(0),
   m_textureData(NULL),
   m_eglImage(NULL),
   m_textureSize(QSize(0, 0))
{
#ifdef DEBUG_TDATA_LEAKS
   count++;
   log_debug("Currently %d texture data around.", count);
#endif // DEBUG_TDATA_LEAKS
}

/*------------------------------------------------------------------------------
|    OMX_TextureData::OMX_TextureData
+-----------------------------------------------------------------------------*/
OMX_TextureData::OMX_TextureData(const OMX_TextureData& textureData) :
   m_textureId(textureData.m_textureId),
   m_textureData(textureData.m_textureData),
   m_eglImage(textureData.m_eglImage),
   m_textureSize(textureData.m_textureSize),
   m_omxBuffer(textureData.m_omxBuffer)
{
#ifdef DEBUG_TDATA_LEAKS
   count++;
   log_debug("Currently %d texture data around.", count);
#endif // DEBUG_TDATA_LEAKS
}

/*------------------------------------------------------------------------------
|    OMX_TextureData::~OMX_TextureData
+-----------------------------------------------------------------------------*/
OMX_TextureData::~OMX_TextureData()
{
#ifdef DEBUG_TDATA_LEAKS
   count--;
   log_debug("Currently %d texture data around.", count);
#endif // DEBUG_TDATA_LEAKS

   if (m_textureData || m_textureId || m_eglImage)
      log_warn("Loosing pointers to GPU data.");
}

/*------------------------------------------------------------------------------
|    OMX_TextureData::freeData
+-----------------------------------------------------------------------------*/
void OMX_TextureData::freeData()
{
   EGLDisplay eglDisplay = get_egl_display();
   assert(eglDisplay);

   // Destroy texture, EGL image and free the buffer.
   if (m_eglImage) {
      log_verbose("Freeing KHR image %p...", m_eglImage);
      if (eglDestroyImageKHR(eglDisplay, m_eglImage) != EGL_TRUE) {
         LOG_ERROR(LOG_TAG, "Failed to destroy EGLImageKHR: %s.", get_egl_errstr());
      }

      m_eglImage = NULL;
   }

   if (m_textureId) {
      log_verbose("Freeing texture %d...", m_textureId);
      glDeleteTextures(1, &m_textureId);

      check_gl_error();

      m_textureId = 0;
   }

   if (m_textureData) {
      log_info("Freeing texture data...");
      delete[] m_textureData;
      m_textureData = NULL;
   }
}
