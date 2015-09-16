/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    04.06.2013
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
#include <QOpenGLContext>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <omx_globals.h>
#include <omx_logging.h>


/*------------------------------------------------------------------------------
|    get_egl_display
+-----------------------------------------------------------------------------*/
EGLDisplay get_egl_display()
{
   QPlatformNativeInterface* nativeInterface =
         QGuiApplicationPrivate::platformIntegration()->nativeInterface();
   Q_ASSERT(nativeInterface);

   EGLDisplay d = nativeInterface->nativeResourceForIntegration("egldisplay");
   if (!d)
      log_warn("Couldn't get EGL display handle.");

   return d;
}

/*------------------------------------------------------------------------------
|    get_egl_context
+-----------------------------------------------------------------------------*/
EGLContext get_global_egl_context()
{
   QOpenGLContext* c = QOpenGLContext::globalShareContext();
   if (!c)
      return (void*)log_warn("Cannot get an OpenGL context.");

   QPlatformNativeInterface* nativeInterface =
         QGuiApplicationPrivate::platformIntegration()->nativeInterface();
   Q_ASSERT(nativeInterface);

   EGLContext eglc =
         nativeInterface->nativeResourceForContext("eglcontext", c);
   if (!eglc)
      log_warn("Couldn't get EGL context from currrent OpenGL context.");

   return eglc;
}

/*------------------------------------------------------------------------------
|    get_egl_errstr
+-----------------------------------------------------------------------------*/
const char* get_egl_errstr(EGLint err)
{
   switch (err) {
   case EGL_SUCCESS:
      return "EGL_SUCCESS";
   case EGL_NOT_INITIALIZED:
      return "EGL_NOT_INITIALIZED";
   case EGL_BAD_ACCESS:
      return "EGL_BAD_ACCESS";
   case EGL_BAD_ALLOC:
      return "EGL_BAD_ALLOC";
   case EGL_BAD_ATTRIBUTE:
      return "EGL_BAD_ATTRIBUTE";
   case EGL_BAD_CONFIG:
      return "EGL_BAD_CONFIG";
   case EGL_BAD_CONTEXT:
      return "EGL_BAD_CONFIG";
   case EGL_BAD_CURRENT_SURFACE:
      return "EGL_BAD_CURRENT_SURFACE";
   case EGL_BAD_DISPLAY:
      return "EGL_BAD_DISPLAY";
   case EGL_BAD_MATCH:
      return "EGL_BAD_MATCH";
   case EGL_BAD_NATIVE_PIXMAP:
      return "EGL_BAD_NATIVE_PIXMAP";
   case EGL_BAD_NATIVE_WINDOW:
      return "EGL_BAD_NATIVE_WINDOW";
   case EGL_BAD_PARAMETER:
      return "EGL_BAD_PARAMETER";
   case EGL_BAD_SURFACE:
      return "EGL_BAD_SURFACE";
   case EGL_CONTEXT_LOST:
      return "EGL_CONTEXT_LOST";
   default:
      return "UNKNOWN";
   }
}

/*------------------------------------------------------------------------------
|    get_egl_errstr
+-----------------------------------------------------------------------------*/
const char* get_egl_errstr()
{
   return get_egl_errstr(eglGetError());
}
