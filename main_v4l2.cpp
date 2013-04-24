/* V4L2 video picture grabber
   Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@infradead.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */

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
#include <vector>
#include <QImage>
#include <lgl_logging.h>

using namespace std;

typedef unsigned char uint8x;
typedef char int8x;

#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum ImageType {
    ImageType_RGB24 = 1,
    ImageType_GRAY8 = 2,
    ImageType_JPEG  = 3
};

struct Image {
    Image() : size_x(0), size_y(0), type(ImageType_GRAY8)
    {}

    int size_x;
    int size_y;

    int type; // @see ImageType

    std::vector<uint8x> pixels;
};

static inline
void saturate(int& value, int min_val, int max_val)
{
  if (value < min_val) value = min_val;
  if (value > max_val) value = max_val;
}

void convert_YUYV_to_RGB24(int size_x, int size_y, const uint8x* YUYV_ptr, Image& out)
{
  const int K1 = int(1.402f * (1 << 16));
  const int K2 = int(0.714f * (1 << 16));
  const int K3 = int(0.334f * (1 << 16));
  const int K4 = int(1.772f * (1 << 16));

  // convert to RGB24
  out.size_x = size_x;
  out.size_y = size_y;
  out.type = ImageType_RGB24;

  out.pixels.resize(size_x * size_y * 3); // 3 bytes per RGB24 pixel

  typedef uint8x T;
  T* out_ptr = &out.pixels[0];
  const int pitch = size_x * 2; // 2 bytes per one YU-YV pixel

  for (int y=0; y<size_y; y++) {
    const uint8x* src = YUYV_ptr + pitch * y;
    for (int x=0; x<size_x*2; x+=4) { // Y1 U Y2 V
      uint8x Y1 = src[x + 0];
      uint8x U  = src[x + 1];
      uint8x Y2 = src[x + 2];
      uint8x V  = src[x + 3];

      int8x uf = U - 128;
      int8x vf = V - 128;

      int R = Y1 + (K1*vf >> 16);
      int G = Y1 - (K2*vf >> 16) - (K3*uf >> 16);
      int B = Y1 + (K4*uf >> 16);

      saturate(R, 0, 255);
      saturate(G, 0, 255);
      saturate(B, 0, 255);

      *out_ptr++ = T(R);
      *out_ptr++ = T(G);
      *out_ptr++ = T(B);

      R = Y2 + (K1*vf >> 16);
      G = Y2 - (K2*vf >> 16) - (K3*uf >> 16);
      B = Y2 + (K4*uf >> 16);

      saturate(R, 0, 255);
      saturate(G, 0, 255);
      saturate(B, 0, 255);

      *out_ptr++ = T(R);
      *out_ptr++ = T(G);
      *out_ptr++ = T(B);
    }

  }
}

struct buffer {
    void   *start;
    size_t length;
};

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

int main(int argc, char **argv)
{
    struct v4l2_format              fmt;
    struct v4l2_buffer              buf;
    struct v4l2_requestbuffers      req;
    enum v4l2_buf_type              type;
    fd_set                          fds;
    struct timeval                  tv;
    int                             r, fd = -1;
    unsigned int                    i, n_buffers;
    char                            *dev_name = "/dev/video0";
    char                            out_name[256];
    FILE                            *fout;
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
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(fd, VIDIOC_S_FMT, &fmt);
    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUV420) {
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
    for (i = 0; i < 20; i++) {
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
            return errno;
        }

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_DQBUF, &buf);

        LOG_VERBOSE(LOG_TAG, "Dumping...");
        Image im;
        convert_YUYV_to_RGB24(640, 480, (uint8x*)(buffers[buf.index].start), im);
        QImage image((unsigned char*)&(im.pixels), 640, 480, QImage::Format_RGB888);
        sprintf(out_name, "out%03d.jpg", i);
        image.save(QString("/home/pi/") + QString(out_name));
        /*fout = fopen(out_name, "w");
        if (!fout) {
            perror("Cannot open image");
            exit(EXIT_FAILURE);
        }
        fprintf(fout, "P6\n%d %d 255\n",
                fmt.fmt.pix.width, fmt.fmt.pix.height);
        fwrite(buffers[buf.index].start, buf.bytesused, 1, fout);
        fclose(fout);*/

        xioctl(fd, VIDIOC_QBUF, &buf);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (i = 0; i < n_buffers; ++i)
        v4l2_munmap(buffers[i].start, buffers[i].length);
    v4l2_close(fd);

    return 0;
}
