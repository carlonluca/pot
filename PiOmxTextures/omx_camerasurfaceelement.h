/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.28.2012
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

#ifndef OMX_CAMERASURFACEELEMENT_H
#define OMX_CAMERASURFACEELEMENT_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QtQuick/QQuickItem>
#include <QOpenGLContext>
#include <QTimer>
#include <QSemaphore>
#include <QImage>

#include "omx_textureproviderqquickitem.h"
#include "lgl_logging.h"

class OMX_SGTexture;


/*------------------------------------------------------------------------------
|    OMX_CameraSurfaceElement class
+-----------------------------------------------------------------------------*/
class OMX_CameraSurfaceElement : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
    OMX_CameraSurfaceElement(QQuickItem* parent = 0);
    ~OMX_CameraSurfaceElement() {}

    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

    QString source() {
        return m_source;
    }

    void setSource(const QString& source) {
        if (!QFile(source).exists()) {
            LOG_WARNING(LOG_TAG, "Source file does not exist.");
            return;
        }
        m_source = source;
        emit sourceChanged(source);
    }

    QSGTextureProvider* textureProvider() const {
        return (QSGTextureProvider*)m_sgtexture;
    }

public slots:
    Q_INVOKABLE void play() {
    }

    Q_INVOKABLE void pause() {
    }

    Q_INVOKABLE void stop() {
    }

signals:
    void sourceChanged(const QString& source);
private:
    void videoAcquire();

    QString m_source;
    GLuint m_texture;
    OMX_TextureProviderQQuickItem* m_provider;
    OMX_SGTexture* m_sgtexture;
    QTimer* m_timer;
    bool m_renderScheduled;

    QSemaphore m_semAcquire;
    QImage m_frame;

    // Used only for init.
    bool m_playScheduled;
};

#endif // OMX_CAMERASURFACEELEMENT_H
