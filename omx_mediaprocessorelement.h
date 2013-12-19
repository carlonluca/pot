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

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QQuickItem>

#include "omx_mediaprocessor.h"


/*------------------------------------------------------------------------------
|    OMX_MediaProcessorElement class
+-----------------------------------------------------------------------------*/
class OMX_MediaProcessorElement : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool autoPlay READ autoPlay WRITE setAutoPlay)
    Q_PROPERTY(long streamLength READ streamLength)
    Q_PROPERTY(long streamPosition READ streamPosition)
public:
    explicit OMX_MediaProcessorElement(QQuickItem* parent = 0);
    ~OMX_MediaProcessorElement();

    QString source();
    void setSource(QString source);

    bool autoPlay();
    void setAutoPlay(bool autoPlay);

    OMX_MediaProcessor* mediaProcessor() {
        return m_mediaProc;
    }

public slots:
    Q_INVOKABLE bool play();
    Q_INVOKABLE bool stop();
    Q_INVOKABLE bool pause();
    Q_INVOKABLE bool seek(long position);

    Q_INVOKABLE long streamLength();
    Q_INVOKABLE long streamPosition();

    void onTextureDataReady(const OMX_TextureData* textureData);

signals:
    void textureReady(const OMX_TextureData* textureId);
    void textureInvalidated();
    void sourceChanged(QString filepath);

    void playbackStarted();
    void playbackCompleted();

protected:
    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

private:
    OMX_MediaProcessor*   m_mediaProc;
    OMX_TextureProviderSh m_texProvider;

    QString m_source;
    bool m_autoPlay = true;
    volatile bool m_pendingOpen;
    OMX_TextureData* m_textureData;

private slots:
    void instantiateMediaProcessor();
    bool openMedia(QString filepath);
};

/*------------------------------------------------------------------------------
|    OMX_MediaProcessorElement::streamLength
+-----------------------------------------------------------------------------*/
inline long OMX_MediaProcessorElement::streamLength() {
    if (m_mediaProc)
        return m_mediaProc->streamLength();
    return -1;
}

/*------------------------------------------------------------------------------
|    OMX_MediaProcessorElement::streamPosition
+-----------------------------------------------------------------------------*/
inline long OMX_MediaProcessorElement::streamPosition() {
    if (m_mediaProc)
        return m_mediaProc->streamPosition();
    return -1;
}

#endif // OMX_MEDIAPROCESSORELEMENT_H
