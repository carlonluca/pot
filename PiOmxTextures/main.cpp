/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.03.2012
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

/*----------------------------------------------------------------------
|    includes
+---------------------------------------------------------------------*/
#include <QApplication>
#include <QQuickView>

#include <bcm_host.h>
#include <signal.h>
#include <execinfo.h>

extern "C" {
#include "libavformat/avformat.h"
}

#include "glwidget.h"
#include "lgl_logging.h"
#include "omx_imageelement.h"
#include "omx_videosurfaceelement.h"
#include "omx_camerasurfaceelement.h"
#include "omx_mediaprocessorelement.h"
#include "omx_audioprocessor.h"
#include "omx_mediaprocessor.h"

#define ENABLE_QML_SAMPLE 1


/*----------------------------------------------------------------------
|    handler
+---------------------------------------------------------------------*/
void handler(int sig) {
  void *array[10];
  size_t size;
  size = backtrace(array, 10);

  // Print out all the frames to stderr.
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

/*----------------------------------------------------------------------
|    definitions
+---------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
#if 0
    signal(SIGSEGV, handler);
#endif

    QApplication a(argc, argv);

    // Registers all the codecs.
    av_register_all();

#ifdef ENABLE_CUBE_SAMPLE
    // Check arguments.
    if (argc < 3) {
        LOG_ERROR(LOG_TAG, "You have to provide a prefix for the textures to use and the path to the video.");
        LOG_ERROR(LOG_TAG, "You'll need 6 textures whose abs path is <prefix><n>.jpg with <n> in [0, 5].");
        LOG_ERROR(LOG_TAG, "Then start the application with <prefix> as the"
                  " first param and <video_path> as the second.");
        return -1;
    }

    // Check file existance.
    for (int i = 0; i < 6; i++) {
        QString filePath = QString("%1%2%3").arg(a.arguments().at(1)).arg(i).arg(".jpg");
        if (!QFile(filePath).exists()) {
            LOG_ERROR(LOG_TAG, "Couldn't find %s.", qPrintable(filePath));
            return -1;
        }
    }
    if (!QFile(a.arguments().at(2)).exists()) {
        LOG_ERROR(LOG_TAG, "Video file does not exist.");
        return -1;
    }

    // Build scene.
    GLWidget widget(a.arguments().at(1), a.arguments().at(2));
    widget.show();
#elif ENABLE_QML_SAMPLE
#ifdef ENABLE_VIDEO_TEST
    qRegisterMetaType<GLuint>("GLuint");
    qmlRegisterType<OMX_ImageElement>("com.luke.qml", 1, 0, "OMXImage");
    qmlRegisterType<OMX_VideoSurfaceElement>("com.luke.qml", 1, 0, "OMXVideoSurface");
    qmlRegisterType<OMX_CameraSurfaceElement>("com.luke.qml", 1, 0, "OMXCameraSurface");
    qmlRegisterType<OMX_MediaProcessorElement>("com.luke.qml", 1, 0, "OMXMediaProcessor");

    QQuickView view;
    view.setSource(QUrl("qrc:///main.qml"));
    view.showFullScreen();
#else
    OMX_AudioProcessor proc;
    proc.play();
#endif // ENABLE_VIDEO_TEST
#endif // ENABLE_CUBE_SAMPLE
    return a.exec();
}
