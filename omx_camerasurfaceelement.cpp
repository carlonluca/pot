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

#include <QSGNode>
#include <QSGOpaqueTextureMaterial>
#include <QtConcurrent/QtConcurrent>
#include <QQuickWindow>

// Private headers.
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

#include "omx_texture.h"
#include "omx_camerasurfaceelement.h"


#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
    void   *start;
    size_t length;
};

/*------------------------------------------------------------------------------
|    OMX_CameraSurfaceElement::OMX_CameraSurfaceElement
+-----------------------------------------------------------------------------*/
OMX_CameraSurfaceElement::OMX_CameraSurfaceElement(QQuickItem* parent) :
    QQuickItem(parent),
    m_renderScheduled(false),
    m_semAcquire(1)
{
    setFlag(ItemHasContents, true);

    // Do nothing.
    LOG_VERBOSE(LOG_TAG, "Starting timer...");
    m_timer = new QTimer;
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(30);
}

/*------------------------------------------------------------------------------
|    OMX_CameraSurfaceElement::updatePaintNode
+-----------------------------------------------------------------------------*/
QSGNode* OMX_CameraSurfaceElement::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    QSGGeometryNode* node = 0;
    QSGGeometry* geometry = 0;

    if (!oldNode) {
        // Create the node.
        node = new QSGGeometryNode;
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
        geometry->setDrawingMode(GL_TRIANGLE_STRIP);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);

        // TODO: Who is freeing this?
        // TODO: I cannot know the texture size here.
        QSGOpaqueTextureMaterial* material = new QSGOpaqueTextureMaterial;
        m_sgtexture = new OMX_SGTexture(0, QSize(640, 480));
        material->setTexture(m_sgtexture);
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);

        QtConcurrent::run(this, &OMX_CameraSurfaceElement::videoAcquire);
    }
    else {
        node = static_cast<QSGGeometryNode*>(oldNode);
        geometry = node->geometry();
        geometry->allocate(4);

        // Update texture in the node if needed.
        QSGOpaqueTextureMaterial* material = (QSGOpaqueTextureMaterial*)node->material();
        QElapsedTimer timer;
        timer.start();
        QSGTexture* texture = window()->createTextureFromImage(m_frame);
        LOG_VERBOSE(LOG_TAG, "Timer tex: %lld.", timer.elapsed());
        material->setTexture(texture);
        m_semAcquire.release();
#if 0
        if (m_texture != (GLuint)material->texture()->textureId()) {
            // TODO: Does setTextureId frees the prev texture?
            // TODO: I should the given the texture size.
            LOG_ERROR(LOG_TAG, "Updating texture to %u!", m_texture);
            material = new QSGOpaqueTextureMaterial;
            m_sgtexture->setTexture(m_texture, QSize(1920, 1080));
        }
#endif
    }

    // Create the vertices and map to texture.
    QRectF bounds = boundingRect();
    QSGGeometry::TexturedPoint2D* vertices = geometry->vertexDataAsTexturedPoint2D();
    vertices[0].set(bounds.x(), bounds.y() + bounds.height(), 0.0f, 0.0f);
    vertices[1].set(bounds.x() + bounds.width(), bounds.y() + bounds.height(), 1.0f, 0.0f);
    vertices[2].set(bounds.x(), bounds.y(), 0.0f, 1.0f);
    vertices[3].set(bounds.x() + bounds.width(), bounds.y(), 1.0f, 1.0f);
    return node;
}

static void xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = v4l2_ioctl(fh, request, arg);
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    if (r == -1) {
        fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void OMX_CameraSurfaceElement::videoAcquire()
{
    LOG_VERBOSE(LOG_TAG, "Started acquisition thread...");
    struct v4l2_format              fmt;
    struct v4l2_buffer              buf;
    struct v4l2_requestbuffers      req;
    enum v4l2_buf_type              type;
    fd_set                          fds;
    struct timeval                  tv;
    int                             r, fd = -1;
    unsigned int                    i, n_buffers;
    char                            *dev_name = (char*)"/dev/video0";
    char                            out_name[256];
#ifdef ENABLE_DUMP
    FILE                            *fout;
#endif
    struct buffer                   *buffers;

    fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        perror("Cannot open device");
        exit(EXIT_FAILURE);
    }

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(fd, VIDIOC_S_FMT, &fmt);
    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
        printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");
        exit(EXIT_FAILURE);
    }
    if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
        printf("Warning: driver is sending image at %dx%d\n",
               fmt.fmt.pix.width, fmt.fmt.pix.height);

    CLEAR(req);
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    xioctl(fd, VIDIOC_REQBUFS, &req);

    buffers = (buffer*)calloc(req.count, sizeof(*buffers));
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        CLEAR(buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        xioctl(fd, VIDIOC_QUERYBUF, &buf);

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
                                             PROT_READ | PROT_WRITE, MAP_SHARED,
                                             fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < n_buffers; ++i) {
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        xioctl(fd, VIDIOC_QBUF, &buf);
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    xioctl(fd, VIDIOC_STREAMON, &type);
    for (i = 0; i < 200; i++) {
        do {
            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);
        } while ((r == -1 && (errno = EINTR)));
        if (r == -1) {
            perror("select");
            return;
        }

        QElapsedTimer timer;
        timer.start();
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        m_semAcquire.acquire();
        xioctl(fd, VIDIOC_DQBUF, &buf);
        printf("Time: %lld.\n", timer.elapsed());

        sprintf(out_name, "out%03d.ppm", i);
#ifdef ENABLE_DUMP
        fout = fopen(out_name, "w");
        if (!fout) {
            perror("Cannot open image");
            exit(EXIT_FAILURE);
        }
        fprintf(fout, "P6\n%d %d 255\n",
                fmt.fmt.pix.width, fmt.fmt.pix.height);
        fwrite(buffers[buf.index].start, buf.bytesused, 1, fout);
        fclose(fout);
#endif
        xioctl(fd, VIDIOC_QBUF, &buf);
        printf("Time: %lld.\n", timer.elapsed());

        m_frame = QImage((uchar*)buffers[buf.index].start, 640, 480, QImage::Format_RGB888);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (i = 0; i < n_buffers; ++i)
        v4l2_munmap(buffers[i].start, buffers[i].length);
    v4l2_close(fd);

    return;
}
