#
# Project: PiOmxTextures
# Author:  Luca Carlon
# Date:    12.03.2012
#
# Copyright (c) 2012, 2013 Luca Carlon. All rights reserved.
#
# This file is part of PiOmxTextures.
#
# PiOmxTextures is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# PiOmxTextures is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
#

QT += core core-private gui gui-private opengl quick quick-private multimedia

SRC=$$PWD
SRC_WRAPPER=$$SRC/omx_wrapper

INCLUDEPATH += \
   $$SRC $$SRC_WRAPPER \
   $$PWD/omxplayer_lib \
   $$PWD/omxplayer_lib/utils \
   $$PWD/omxplayer_lib/linux \
   $$PWD/ilclient \
   $$PWD/../3rdparty/LightLogger
INCLUDEPATH += \
   $$PWD/../3rdparty/LightLogger \
	$$PWD/../3rdparty/LightSmartPtr

linux-rasp-pi-g++ {
	message("Building for RPi1...");
	FFMPEG_BUILD_DIR = ffmpeg_pi1
}

linux-rasp-pi2-g++ {
	message("Building for RPi2...");
	FFMPEG_BUILD_DIR = ffmpeg_pi2
}

LIBS += -lopenmaxil -lGLESv2 -lEGL -lbcm_host -lvcos -lrt -lv4l2
INCLUDEPATH += $$PWD/../3rdparty/ffmpeg/$$FFMPEG_BUILD_DIR/include
#LIBS += -lavformat -lavcodec -lavutil
# Internal
DEFINES += CONFIG_INCLUDE_FFMPEG
contains(DEFINES, CONFIG_INCLUDE_FFMPEG) {
LIBS += $$PWD/../3rdparty/ffmpeg/$$FFMPEG_BUILD_DIR/lib/libavformat.a \
	$$PWD/../3rdparty/ffmpeg/$$FFMPEG_BUILD_DIR/lib/libavcodec.a \
	$$PWD/../3rdparty/ffmpeg/$$FFMPEG_BUILD_DIR/lib/libavutil.a \
	$$PWD/../3rdparty/ffmpeg/$$FFMPEG_BUILD_DIR/lib/libswscale.a \
	$$PWD/../3rdparty/ffmpeg/$$FFMPEG_BUILD_DIR/lib/libswresample.a \
   -lz -lssl -lcrypto -lsmbclient -lssh -lbz2 -lpcre
}
else {
LIBS += -L$$PWD/../3rdparty/ffmpeg/$$FFMPEG_BUILD_DIR/lib \
   -lavformat -lavcodec -lavutil -lswscale -lswresample
}

# For omxplayer.
#LIBS += -lfreetype -lWFC -lpcre
#INCLUDEPATH += /usr/include/freetype2
CONFIG += link_pkgconfig
PKGCONFIG += freetype2

# Flags used by hello_pi examples:
#-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
# -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE
# -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX
# -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST
# -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi

DEFINES += __STDC_CONSTANT_MACROS \
   __STDC_LIMIT_MACROS \
   TARGET_POSIX \
   TARGET_LINUX \
   PIC \
   _REENTRANT \
   _LARGEFILE64_SOURCE \
   _FILE_OFFSET_BITS=64 \
   HAVE_LIBOPENMAX=2 \
   OMX \
   OMX_SKIP64BIT \
   USE_EXTERNAL_OMX \
   HAVE_LIBBCM_HOST \
   USE_EXTERNAL_LIBBCM_HOST \
   USE_VCHIQ_ARM \
   HAVE_OMXLIB

# This is related to modifications to omxplayer merged to PiOmxTextures that seemed
# to perform badly. After some revisions these seem to be acceptable and were kept
# to keep sources in sync with omxplayer code.
DEFINES += ENABLE_IMPROVED_BUFFERING

# To enable subtitles.
#DEFINES += ENABLE_SUBTITLES

# Enable this to monitor performance of the main loop in omx_mediaprocessor.h. Can
# also be enabled in omx_mediaprocessor.h.
#DEFINES += ENABLE_PROFILE_MAIN_LOOP

# This enables pause/resume implmentation in the main loop in omx_mediaprocessor.h.
# This code was removed because too much computation in that loop seems to cause
# too much CPU load which causes repeated pause/resume. A couple of ms are spared
# avoiding the computation and only small interruptions seem to give a better result.
# Can also be enabled in omx_mediaprocessor.h.
#DEFINES += ENABLE_PAUSE_FOR_BUFFERING

# This enables logs coming from omxplayer core.
#DEFINES += ENABLE_OMXPLAYER_LOGS

# Define this to enable watchdog.
#DEFINES += OMX_LOCK_WATCHDOG

# For omxplayer.
QMAKE_CXXFLAGS += -std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS             \
   -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE               \
   -D_FILE_OFFSET_BITS=64 -DHAVE_CMAKE_CONFIG -D__VIDEOCORE4__                          \
   -U_FORTIFY_SOURCE -DUSE_EXTERNAL_FFMPEG  -DHAVE_LIBAVCODEC_AVCODEC_H                 \
   -DHAVE_LIBAVUTIL_OPT_H -DHAVE_LIBSWRESAMPLE_SWRESAMPLE_H -DHAVE_LIBAVUTIL_MEM_H      \
   -DHAVE_LIBAVUTIL_AVUTIL_H                                                            \
   -DHAVE_LIBAVFORMAT_AVFORMAT_H -DHAVE_LIBAVFILTER_AVFILTER_H -DOMX -DOMX_SKIP64BIT    \
   -ftree-vectorize -DUSE_EXTERNAL_OMX -DTARGET_RASPBERRY_PI -DUSE_EXTERNAL_LIBBCM_HOST \
   -Wno-deprecated-declarations -Wno-missing-field-initializers -Wno-ignored-qualifiers \
   -Wno-psabi -Wno-unused-parameter

#SOURCES += \
#   $$SRC/glwidget.cpp \
#   $$SRC/openmaxiltextureloader.cpp \
#   $$SRC_WRAPPER/Event.cpp \
#   $$SRC_WRAPPER/Locker.cpp \
#   $$SRC_WRAPPER/OMXComponent.cpp \
#   $$SRC_WRAPPER/OMX_Core.cpp \
#   $$SRC_WRAPPER/omxtunnel.cpp

#HEADERS += \
#   $$SRC/glwidget.h \
#   $$SRC/openmaxiltextureloader.h \
#   $$SRC_WRAPPER/Event.h \
#   $$SRC_WRAPPER/IEvent.h \
#   $$SRC_WRAPPER/ILocker.h \
#   $$SRC_WRAPPER/Locker.h \
#   $$SRC_WRAPPER/OMXComponent.h \
#   $$SRC_WRAPPER/OMX_Core.h \
#   $$SRC_WRAPPER/omxtunnel.h

SOURCES += \
    $$SRC/openmaxiltextureloader.cpp \
    $$SRC_WRAPPER/Locker.cpp \
    $$SRC_WRAPPER/Event.cpp \
    $$SRC_WRAPPER/OMXComponent.cpp \
#    glwidget.cpp \
#    ilclient/ilcore.c \
#    ilclient/ilclient.c \
#    video.cpp \
    $$SRC_WRAPPER/omxtunnel.cpp \
    $$SRC/omx_imageelement.cpp \
    $$SRC/omx_videosurfaceelement.cpp \
#    omx_texturedelement.cpp \
#    omx_videoprocessor.cpp \     # [1]
    $$SRC/omx_camerasurfaceelement.cpp \
    $$SRC/omx_audioprocessor.cpp \
    $$SRC/omx_mediaprocessor.cpp \
#    omx_videograph.cpp \         # [1]
    $$SRC_WRAPPER/OMX_Core.cpp \
    $$SRC/omx_mediaprocessorelement.cpp \
    $$SRC/omx_globals.cpp \
    $$SRC/omx_textureprovider.cpp \
    $$SRC/omx_playeraudio.cpp \
    $$SRC/omx_reader.cpp \
    $$SRC/ilclient/* \
    $$SRC/omx_staticconf.cpp \
    $$SRC/omx_watchdog.cpp

# This is the PiOmxTextures implementation of the logging class
# in omxplayer.
SOURCES += $$SRC/omx_omxplayer_logging.cpp

SOURCES += \
    $$SRC/omxplayer_lib/Srt.cpp \
    $$SRC/omxplayer_lib/Unicode.cpp \
    $$SRC/omxplayer_lib/SubtitleRenderer.cpp \
    $$SRC/omxplayer_lib/OMXVideo.cpp \
    $$SRC/omxplayer_lib/OMXThread.cpp \
    $$SRC/omxplayer_lib/OMXSubtitleTagSami.cpp \
    $$SRC/omxplayer_lib/OMXStreamInfo.cpp \
    $$SRC/omxplayer_lib/OMXReader.cpp \
    $$SRC/omxplayer_lib/OMXPlayerVideo.cpp \
    $$SRC/omxplayer_lib/OMXPlayerSubtitles.cpp \
    $$SRC/omxplayer_lib/OMXPlayerAudio.cpp \
#    omxplayer_lib/omxplayer.cpp \
    $$SRC/omxplayer_lib/OMXOverlayCodecText.cpp \
    $$SRC/omxplayer_lib/OMXCore.cpp \
    $$SRC/omxplayer_lib/OMXClock.cpp \
    $$SRC/omxplayer_lib/OMXAudioCodecOMX.cpp \
    $$SRC/omxplayer_lib/OMXAudio.cpp \
    $$SRC/omxplayer_lib/File.cpp \
    $$SRC/omxplayer_lib/DynamicDll.cpp \
    $$SRC/omxplayer_lib/BitstreamConverter.cpp \
    $$SRC/omxplayer_lib/linux/XMemUtils.cpp \
    $$SRC/omxplayer_lib/linux/RBP.cpp \
    $$SRC/omxplayer_lib/utils/RegExp.cpp \
    $$SRC/omxplayer_lib/utils/PCMRemap.cpp

# Define log.cpp only if you want to use the omxplayer CLog
# implementation. Otherwise use omx_omxplayer_loggin.cpp.
#SOURCES += omxplayer_lib/utils/log.cpp

HEADERS  += \
    $$SRC/openmaxiltextureloader.h \
    $$SRC_WRAPPER/Locker.h \
    $$SRC_WRAPPER/ILocker.h \
    $$SRC_WRAPPER/IEvent.h \
    $$SRC_WRAPPER/Event.h \
    $$SRC_WRAPPER/OMXComponent.h \
    $$SRC_WRAPPER/MyDeleter.h \
#    glwidget.h \
#    ilclient/ilclient.h \
    $$SRC/omx_wrapper/omxtunnel.h \
    $$SRC/omx_imageelement.h \
    $$SRC/omx_videosurfaceelement.h \
#    omx_texturedelement.h \    # [1]
#    omx_videoprocessor.h \
    $$SRC/omx_camerasurfaceelement.h \
    $$SRC/omx_texture.h \
    $$SRC/omx_qthread.h \
    $$SRC/omx_audioprocessor.h \
    $$SRC/omx_globals.h \
    $$SRC/omx_mediaprocessor.h \
#    omx_videograph.h \         # [1]
    $$SRC/omxplayer_lib/DllOMX.h \
    $$SRC/omxplayer_lib/DllAvFormat.h \
    $$SRC/omxplayer_lib/BitstreamConverter.h \
    $$SRC/omxplayer_lib/OMXSubtitleTagSami.h \
    $$SRC/omxplayer_lib/system.h \
    $$SRC/omx_wrapper/OMX_Core.h \
    $$SRC/omxplayer_lib/DllSwResample.h \
    $$SRC/omxplayer_lib/DllAvUtil.h \
    $$SRC/omxplayer_lib/DllAvCodec.h \
    $$SRC/omx_mediaprocessorelement.h \
    $$SRC/omx_textureprovider.h \
    $$SRC/omx_playeraudio.h \
    $$SRC/omx_reader.h \
    $$SRC/ilclient/* \
    $$SRC/omx_logging.h \
    $$SRC/omx_staticconf.h \
    $$SRC/omx_watchdog.h

HEADERS += \
    $$SRC/omxplayer_lib/Unicode.h \
    $$SRC/omxplayer_lib/SubtitleRenderer.h \
    $$SRC/omxplayer_lib/OMXVideo.h \
    $$SRC/omxplayer_lib/OMXThread.h \
    $$SRC/omxplayer_lib/OMXStreamInfo.h \
    $$SRC/omxplayer_lib/OMXReader.h \
    $$SRC/omxplayer_lib/OMXPlayerVideo.h \
    $$SRC/omxplayer_lib/OMXPlayerSubtitles.h \
    $$SRC/omxplayer_lib/OMXPlayerAudio.h \
    $$SRC/omxplayer_lib/OMXCore.h \
    $$SRC/omxplayer_lib/OMXClock.h \
    $$SRC/omxplayer_lib/OMXAudioCodecOMX.h \
    $$SRC/omxplayer_lib/OMXAudio.h \
    $$SRC/omxplayer_lib/File.h \
    $$SRC/omxplayer_lib/DynamicDll.h \
    $$SRC/omxplayer_lib/linux/XMemUtils.h \
    $$SRC/omxplayer_lib/linux/RBP.h \
    $$SRC/omxplayer_lib/utils/RegExp.h \
    $$SRC/omxplayer_lib/utils/PCMRemap.h \
    $$SRC/omxplayer_lib/utils/log.h
