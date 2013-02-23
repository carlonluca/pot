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

#ifndef PO_VIDEORENDERER_H
#define PO_VIDEORENDERER_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QtQuick/QQuickItem>
#include <QOpenGLContext>


/*------------------------------------------------------------------------------
|    OMX_ImageElement class
+-----------------------------------------------------------------------------*/
class OMX_ImageElement : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
    OMX_ImageElement(QQuickItem* parent = 0);
    ~OMX_ImageElement();

    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

    QString source() {
        return m_source;
    }

    void setSource(const QString& source) {
        // Reset the source path, emit the signal and set the texture
        // to 0 to notify the need for it to be updated again.
        this->m_source = source;
        emit sourceChanged(source);
        m_texture = 0;
        update();
    }

signals:
    void sourceChanged(const QString& source);

private:
    void updateTexture();

    QString m_source;
    GLuint m_texture;
};

#endif // PO_VIDEORENDERER_H
