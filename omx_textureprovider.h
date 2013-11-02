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

#endif // OMX_TEXTUREPROVIDERQQUICKITEM_H
