/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.16.2012
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

#ifndef PO_VIDEOSURFACEELEMENT_H
#define PO_VIDEOSURFACEELEMENT_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QtQuick/QQuickItem>
#include <QOpenGLContext>
#include <QTimer>

#include "lgl_logging.h"
#include "omx_mediaprocessorelement.h"
#include "omx_textureproviderqquickitem.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
class OMX_SGTexture;
class OMX_MediaProcessor;


/*------------------------------------------------------------------------------
|    OMX_VideoSurfaceElement class
+-----------------------------------------------------------------------------*/
class OMX_VideoSurfaceElement : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* source READ source WRITE setSource NOTIFY sourceChanged)
public:
    OMX_VideoSurfaceElement(QQuickItem* parent = 0);
    ~OMX_VideoSurfaceElement();

    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

    void setSource(QObject* source) {
        LOG_VERBOSE(LOG_TAG, "Source was set: %x.", (unsigned int)source);
        m_source = (OMX_MediaProcessorElement*)source;
        connect(m_source, SIGNAL(textureReady(GLuint)), this, SLOT(onTextureChanged(GLuint)), Qt::UniqueConnection);
        emit sourceChanged(source);
    }

    QObject* source() {
        return m_source;
    }

    QSGTextureProvider* textureProvider() const {
        return (QSGTextureProvider*)m_sgtexture;
    }

signals:
    void sourceChanged(const QObject* source);

public slots:
    void onTextureChanged(const GLuint& textureId);

private:
    GLuint m_texture;
    OMX_MediaProcessorElement* m_source;
#ifdef ENABLE_VIDEO_PROCESSOR
    OMX_VideoProcessor* m_videoProc;
#else
    OMX_MediaProcessor* m_mediaProc;
#endif
    OMX_SGTexture* m_sgtexture;
    QTimer* m_timer;
};

#endif // PO_VIDEOSURFACEELEMENT_H
