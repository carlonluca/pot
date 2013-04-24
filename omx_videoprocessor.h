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

#ifndef OMX_VIDEOPROCESSOR_H
#define OMX_VIDEOPROCESSOR_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QObject>
#include <QString>
#include <QMutex>
#include <QThread>
#include <QSGTexture>
#include <QSemaphore>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <IL/OMX_Core.h>
#include <GLES2/gl2.h>

#include <assert.h>

#include "omx_textureproviderqquickitem.h"
#include "omx_texture.h"
#include "omx_qthread.h"
#include "lgl_logging.h"


/*------------------------------------------------------------------------------
|    OMX_VideoProcessor class
+-----------------------------------------------------------------------------*/
class OMX_VideoProcessor : public QObject
{
    Q_OBJECT
public:
    enum State {
        STATE_STOPPED,
        STATE_INACTIVE,
        STATE_PAUSED,
        STATE_PLAYING
    };

    enum Error {
        ERROR_RESOURCE_MISSING,
        ERROR_RESOURCE_READ_FAILED,
        ERROR_FAILED_DECODING
    };

    explicit OMX_VideoProcessor(
            EGLDisplay eglDisplay,
            EGLContext eglContext,
            OMX_TextureProvider* provider,
            QObject* parent = 0
            );
    ~OMX_VideoProcessor();

    // These are blocking. Only return when the request has been
    // performed.
    bool setVideoPath(const QString& videoAbsolutePath);
    bool play();
    bool stop();
    bool pause();

    inline uint getCurrentPosition() {return 0;}
    inline State state() {return m_state;}

signals:
    void playbackFinished();
    void playbackPaused();
    void playbackStopped();
    void playbackFailed(const Error& error);

    //void textureReady(const OMX_VideoProcessorTexture& texture);
    void textureReady(const uint& textureId);

private slots:
    void videoDecoding();

private:
    void doVideoDecoding();
    bool checkCurrentThread();
    bool instantiateTexture(QSize size);

    QString m_videoAbsolutePath;
    State m_state;

    QMutex m_sendCmd;
    QSemaphore m_pendingCmd;

    OMX_QThread m_thread;
    OMX_Texture m_texture;
    OMX_TextureProvider* m_provider;

    EGLDisplay m_eglDisplay;
    EGLContext m_eglContext;

    OMX_BUFFERHEADERTYPE* m_eglBuffer;

    QSemaphore m_playLock;

    friend class OMX_TextureProvider;
};

#endif // OMX_VIDEOPROCESSOR_H
