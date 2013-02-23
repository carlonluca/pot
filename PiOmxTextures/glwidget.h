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

#ifndef GLWIDGET_H
#define GLWIDGET_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QtWidgets>
#include <QGLWidget>
#include <omx_videoprocessor.h>

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
class QGLShaderProgram;
class GLWidget;

/*------------------------------------------------------------------------------
|    OMX_TextureProviderImpl class
+-----------------------------------------------------------------------------*/
class OMX_TextureProviderQGLWidget : public OMX_TextureProvider
{
    Q_OBJECT
public:
    OMX_TextureProviderQGLWidget(GLWidget* widget) : OMX_TextureProvider() {
        m_parent = widget;
    }

public slots:
    GLuint instantiateTexture(QSize size);
    void freeTexture(GLuint textureId);

private:
    GLWidget* m_parent;
};

/*------------------------------------------------------------------------------
|    GLWidget class
+-----------------------------------------------------------------------------*/
class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(
            QString prefix,
            QString videoPath,
            QWidget* parent = 0,
            QGLWidget* shareWidget = 0
            );
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void rotateBy(int xAngle, int yAngle, int zAngle);
    void setClearColor(const QColor &color);

signals:
    void clicked();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void onTextureChanged(const uint& texture);

private:
    void makeObject();
    void loadWithOmx();

    QColor clearColor;
    QPoint lastPos;
    int xRot;
    int yRot;
    int zRot;
    GLuint textures[6];
    QVector<QVector3D> vertices;
    QVector<QVector2D> texCoords;
#ifdef QT_OPENGL_ES_2
    QGLShaderProgram* program;
#endif
    QString prefix;
    QString videoPath;

    OMX_VideoProcessor* m_videoProc;
    OMX_TextureProviderQGLWidget* m_provider;

    friend class OMX_TextureProviderQGLWidget;
};

#endif
