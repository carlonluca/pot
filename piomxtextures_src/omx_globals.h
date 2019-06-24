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

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QString>
#include <QRect>
#include <EGL/egl.h>

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#define LIKELY(x)       __builtin_expect((x), 1)
#define UNLIKELY(x)     __builtin_expect((x), 0)

EGLDisplay get_egl_display();
EGLContext get_global_egl_context();

const char* get_egl_errstr(EGLint err);
const char* get_egl_errstr();

/*------------------------------------------------------------------------------
|    geometry_string
+-----------------------------------------------------------------------------*/
inline QString geometry_string(const QRect& r)
{
    return QStringLiteral("%1 %2 %3 %4")
            .arg(r.x())
            .arg(r.y())
            .arg(r.right())
            .arg(r.bottom());
}

#endif // OMX_GLOBALS_H
