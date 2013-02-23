/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Company: -
 * Date:    05.12.2011
 *
 * Copyright (c) 2012 Luca Carlon. All rights reserved.
 */

#include <QtWidgets>
#include <QtOpenGL>
#include <QtConcurrent/QtConcurrentRun>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include "openmaxiltextureloader.h"

#include "glwidget.h"
#include "lgl_logging.h"


GLWidget::GLWidget(QString prefix, QString videoPath, QWidget *parent, QGLWidget *shareWidget)
    : QGLWidget(parent, shareWidget)
{
    clearColor = Qt::black;
    xRot = 0;
    yRot = 0;
    zRot = 0;
#ifdef QT_OPENGL_ES_2
    program = 0;
#endif
    this->prefix    = prefix;
    this->videoPath = videoPath;

    // Update timer.
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->setSingleShot(false);
    timer->start(30);

    // Provider of textures for OMX_VideoProcessor.
    m_provider = new OMX_TextureProviderQGLWidget(this);
}

GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(200, 200);
}

void GLWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
    xRot += xAngle;
    yRot += yAngle;
    zRot += zAngle;
    updateGL();
}

void GLWidget::setClearColor(const QColor &color)
{
    clearColor = color;
    updateGL();
}

void GLWidget::initializeGL()
{
    makeObject();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
#ifndef QT_OPENGL_ES_2
    glEnable(GL_TEXTURE_2D);
#endif

#ifdef QT_OPENGL_ES_2

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

    QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec4 texCoord;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);
    const char *fsrc =
        "uniform sampler2D texture;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texc.st);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    program = new QGLShaderProgram(this);
    program->addShader(vshader);
    program->addShader(fshader);
    program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
    program->link();

    program->bind();
    program->setUniformValue("texture", 0);

#endif
}

/*------------------------------------------------------------------------------
|    OpenMAXILTextureLoader::getEGLImage
+-----------------------------------------------------------------------------*/
inline
EGLImageKHR getEGLImage(
        OMX_U32 width,
        OMX_U32 height,
        EGLDisplay eglDisplay,
        EGLContext eglContext,
        GLuint& texture
        )
{
    EGLint attr[] = { EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE};

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    GLubyte* pixel = new GLubyte[width*height*4];
    memset(pixel, 0x0f, sizeof(GLubyte)*width*height*4);  // to have a grey texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    EGLImageKHR eglImage = eglCreateImageKHR(
                eglDisplay,
                eglContext,
                EGL_GL_TEXTURE_2D_KHR,
                (EGLClientBuffer)texture,
                attr
                );
    LOG_VERBOSE(LOG_TAG, "EGLImage:      %x.", (unsigned int)eglImage);
    LOG_VERBOSE(LOG_TAG, "Client buffer: %x.", texture);
    EGLint eglErr = eglGetError();
    if(eglErr != EGL_SUCCESS) {
        LOG_ERROR(LOG_TAG, "Failed to create KHR image: %d.", eglErr);
        return 0;
    }

    LOG_VERBOSE(LOG_TAG, "Successfully created KHR image: %x.", (unsigned int)eglImage);
    return eglImage;
}
void GLWidget::paintGL()
{
    qglClearColor(clearColor);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if !defined(QT_OPENGL_ES_2)

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -10.0f);
    glRotatef(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

    glVertexPointer(3, GL_FLOAT, 0, vertices.constData());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords.constData());
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#else

    QMatrix4x4 m;
    m.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 4.0f, 15.0f);
    m.translate(0.0f, 0.0f, -10.0f);
    m.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    m.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

    program->setUniformValue("matrix", m);
    program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    program->setAttributeArray
        (PROGRAM_VERTEX_ATTRIBUTE, vertices.constData());
    program->setAttributeArray
        (PROGRAM_TEXCOORD_ATTRIBUTE, texCoords.constData());

#endif

    for (int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
    }
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

#if !defined(QT_OPENGL_ES_2)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifndef QT_OPENGL_ES
    glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
#else
    glOrthof(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
#endif
    glMatrixMode(GL_MODELVIEW);
#endif
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        rotateBy(8 * dy, 8 * dx, 0);
    } else if (event->buttons() & Qt::RightButton) {
        rotateBy(8 * dy, 0, 8 * dx);
    }
    lastPos = event->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    emit clicked();
}

void GLWidget::onTextureChanged(const uint &texture)
{
    LOG_VERBOSE(LOG_TAG, "Setting textures.");
    for (int i = 0; i < 6; i++)
        this->textures[i] = texture;
}

EGLImageKHR eglImageVideo;
GLuint OMX_TextureProviderQGLWidget::instantiateTexture(QSize size)
{
    m_parent->makeCurrent();
    QPlatformNativeInterface* nativeInterface = QGuiApplicationPrivate::platformIntegration()->nativeInterface();
    Q_ASSERT(nativeInterface);
    EGLDisplay eglDisplay = nativeInterface->nativeResourceForIntegration("egldisplay");
    EGLContext eglContext = nativeInterface->nativeResourceForContext("eglcontext", QOpenGLContext::currentContext());
    GLuint texture;
    eglImageVideo = getEGLImage(size.width(), size.height(), eglDisplay, eglContext, texture);
    return texture;
}

void OMX_TextureProviderQGLWidget::freeTexture(GLuint textureId)
{
    // TODO: Implementation is missing.
}

void GLWidget::makeObject()
{
    static const int coords[6][4][3] = {
        { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
        { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
        { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
        { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
        { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
        { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };

    QElapsedTimer timer;
    timer.start();
#ifndef DISABLED_OPENMAX
    //loadWithOmx();

    QPlatformNativeInterface* nativeInterface = QGuiApplicationPrivate::platformIntegration()->nativeInterface();
    Q_ASSERT(nativeInterface);
    EGLDisplay eglDisplay = nativeInterface->nativeResourceForIntegration("egldisplay");
    EGLContext eglContext = nativeInterface->nativeResourceForContext("eglcontext", QOpenGLContext::currentContext());
#if 0
    eglImageVideo = getEGLImage(1920, 1080, eglDisplay, eglContext, textures[0]);
#endif
    for (int i = 0; i < 5; i++)
        textures[i] = 0;
    //QtConcurrent::run(video_decode_test, videoPath, eglImageVideo, eglDisplay);
    m_videoProc = new OMX_VideoProcessor(eglDisplay, eglContext, m_provider);
    connect(m_videoProc, SIGNAL(textureReady(uint)), this, SLOT(onTextureChanged(uint)));
    m_videoProc->setVideoPath("/home/pi/out.h264");
    m_videoProc->play();
#else
    for (int i = 0; i < 6; i++) {
        QPixmap pixmap(QString("%1%2.jpg").arg(prefix).arg(i));
        if (pixmap.isNull())
            LOG_ERROR(LOG_TAG, "Failed to load image!");
        textures[i] = bindTexture(pixmap, GL_TEXTURE_2D, GL_RGBA);
    }
#endif
    LOG_INFORMATION(LOG_TAG, "Elapsed: %lld.", timer.elapsed());

    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 4; ++j) {
            texCoords.append
                (QVector2D(j == 0 || j == 3, j == 0 || j == 1));
            vertices.append
                (QVector3D(0.2 * coords[i][j][0], 0.2 * coords[i][j][1],
                           0.2 * coords[i][j][2]));
        }
    }
}

void GLWidget::loadWithOmx()
{
    LOG_VERBOSE(LOG_TAG, "Loading with OMX.");
    QPlatformNativeInterface* nativeInterface = QGuiApplicationPrivate::platformIntegration()->nativeInterface();
    Q_ASSERT(nativeInterface);
    EGLDisplay eglDisplay = nativeInterface->nativeResourceForIntegration("egldisplay");
    EGLContext eglContext = nativeInterface->nativeResourceForContext("eglcontext", QOpenGLContext::currentContext());

    for (int i = 5; i < 6; i++) {
        QString fileAbsPath = QString("%1%2.jpg").arg(prefix).arg(i);
        OpenMAXILTextureLoader* omTextureLoader = OpenMAXILTextureLoader::intance();
        if (!omTextureLoader->loadTextureFromImage(fileAbsPath, eglDisplay, eglContext, textures[i])) {
            LOG_ERROR(LOG_TAG, "Failed to load image.");
        }
        else {
            LOG_INFORMATION(LOG_TAG, "Image %s successfully decoded and loaded.", qPrintable(fileAbsPath));
        }
    }
}
