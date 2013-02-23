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

#ifndef OMX_MEDIAPROCESSORELEMENT_H
#define OMX_MEDIAPROCESSORELEMENT_H

#include <QQuickItem>

#include "omx_mediaprocessor.h"


class OMX_MediaProcessorElement : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
    explicit OMX_MediaProcessorElement(QQuickItem* parent = 0);
    ~OMX_MediaProcessorElement();

    QString source();
    void setSource(QString source);

    OMX_MediaProcessor* mediaProcessor() {
        return m_mediaProc;
    }

public slots:
    Q_INVOKABLE bool play();
    Q_INVOKABLE bool stop();
    Q_INVOKABLE bool pause();
    Q_INVOKABLE bool seek(long millis);
    Q_INVOKABLE long currentPosition();

signals:
    void textureReady(const GLuint& textureId);
    void sourceChanged(QString filepath);

    void playbackStarted();
    void playbackCompleted();

protected:
    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

private:
    bool openMedia(QString filepath);

    OMX_MediaProcessor* m_mediaProc;
    OMX_TextureProvider* m_texProvider;

    GLuint m_textureId; // TODO: Other info might be needed.

    QString m_source;
};

#endif // OMX_MEDIAPROCESSORELEMENT_H
