/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.31.2012
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
 * along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OMX_GLOBALS_H
#define OMX_GLOBALS_H

#include <QOpenGLContext>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <EGL/egl.h>

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#define OMX_INIT_STRUCTURE(a)                            \
    memset(&(a), 0, sizeof(a));                          \
    (a).nSize                    = sizeof(a);            \
    (a).nVersion.nVersion        = OMX_VERSION;


/*------------------------------------------------------------------------------
|    get_egl_display
+-----------------------------------------------------------------------------*/
inline
EGLDisplay get_egl_display()
{
    QPlatformNativeInterface* nativeInterface = QGuiApplicationPrivate::platformIntegration()->nativeInterface();
    Q_ASSERT(nativeInterface);
    return nativeInterface->nativeResourceForIntegration("egldisplay");
}

/*------------------------------------------------------------------------------
|    get_egl_context
+-----------------------------------------------------------------------------*/
inline
EGLContext get_egl_context()
{
    QPlatformNativeInterface* nativeInterface = QGuiApplicationPrivate::platformIntegration()->nativeInterface();
    Q_ASSERT(nativeInterface);
    return nativeInterface->nativeResourceForContext("eglcontext", QOpenGLContext::currentContext());
}

#endif // OMX_GLOBALS_H
