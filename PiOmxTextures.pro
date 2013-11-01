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

# On the command line you can manually speciy some params.
# qmake "DEFINES+=[CONFIG_APP|CONFIG_LIB] [CONFIG_INCLUDE_FFMPEG]"

# Either CONFIG_LIB or CONFIG_APP.
#DEFINES += CONFIG_LIB
#DEFINES += CONFIG_APP
!contains(DEFINES, CONFIG_APP):!contains(DEFINES, CONFIG_LIB) {
error("Either add CONFIG_APP or CONFIG_LIB to DEFINES.")
}

QT += core core-private gui gui-private opengl quick quick-private

TARGET   = PiOmxTextures
contains(DEFINES, CONFIG_LIB) {
TEMPLATE = lib

# Install headers.
headers.files  = \
   omx_mediaprocessor.h \
   omx_textureprovider.h \
   omx_qthread.h \
   omx_globals.h \
   lgl_logging.h
headers.path   = $$OUT_PWD/piomxtextures

omxplayer_headers.files = \
   omxplayer_lib/OMXStreamInfo.h
omxplayer_headers.path = $$OUT_PWD/piomxtextures/omxplayer_lib

#omxplayer_linux_headers.files = omxplayer_lib/linux/RBP.h
#omxplayer_linux_headers.path  = $$OUT_PWD/piomxtextures/omxplayer_lib/linux

INSTALLS += headers omxplayer_headers
}
else:contains(DEFINES, CONFIG_APP) {
TEMPLATE = app
}
else {
error("Either config as app or lib.");
}

# External
LIBS += -lopenmaxil -lGLESv2 -lEGL -lbcm_host -lvcos -lrt -lv4l2
#LIBS += -lavformat -lavcodec -lavutil
# Internal
contains(DEFINES, CONFIG_INCLUDE_FFMPEG) {
LIBS += $$_PRO_FILE_PWD_/3rdparty/ffmpeg/lib/libavformat.a \
   $$_PRO_FILE_PWD_/3rdparty/ffmpeg/lib/libavcodec.a \
   $$_PRO_FILE_PWD_/3rdparty/ffmpeg/lib/libavutil.a \
   $$_PRO_FILE_PWD_/3rdparty/ffmpeg/lib/libswscale.a \
   $$_PRO_FILE_PWD_/3rdparty/ffmpeg/lib/libswresample.a
}
else {
LIBS += -L$$_PRO_FILE_PWD_/3rdparty/ffmpeg/lib \
   -lavformat -lavcodec -lavutil -lswscale -lswresample
}
INCLUDEPATH += $$_PRO_FILE_PWD_/3rdparty/ffmpeg/include
# Add this if building with old firmware.
INCLUDEPATH += /opt/rpi/sysroot/opt/vc/include/interface/vmcs_host/linux
# For omxplayer.
LIBS += -lfreetype -lWFC -lpcre
INCLUDEPATH += /usr/include/freetype2

CONFIG += link_pkgconfig
PKGCONFIG += freetype2

INCLUDEPATH += \
   omx_wrapper \
   ilclient \
   3rdparty/include

INCLUDEPATH += \
   omxplayer_lib \
   omxplayer_lib/utils \
   omxplayer_lib/linux

VERSION = 4.2.0

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

# Macro definitions
#DEFINES += LOG_LEVEL_DEBUG
DEFINES += VERBOSE
DEFINES += ENABLE_VIDEO_TEST
DEFINES += ENABLE_MEDIA_PROCESSOR

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

# For omxplayer.
QMAKE_CXXFLAGS += -std=c++0x -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS             \
   -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE               \
   -D_FILE_OFFSET_BITS=64 -DHAVE_CMAKE_CONFIG -D__VIDEOCORE4__                          \
   -U_FORTIFY_SOURCE -DUSE_EXTERNAL_FFMPEG  -DHAVE_LIBAVCODEC_AVCODEC_H                 \
   -DHAVE_LIBAVUTIL_OPT_H -DHAVE_LIBSWRESAMPLE_SWRESAMPLE_H -DHAVE_LIBAVUTIL_MEM_H      \
   -DHAVE_LIBAVUTIL_AVUTIL_H                                                            \
   -DHAVE_LIBAVFORMAT_AVFORMAT_H -DHAVE_LIBAVFILTER_AVFILTER_H -DOMX -DOMX_SKIP64BIT    \
   -ftree-vectorize -DUSE_EXTERNAL_OMX -DTARGET_RASPBERRY_PI -DUSE_EXTERNAL_LIBBCM_HOST \
   -Wno-deprecated-declarations -Wno-missing-field-initializers -Wno-ignored-qualifiers \
   -Wno-psabi -Wno-unused-parameter

# [1]: part of implementation without omxplayer.

contains(DEFINES, CONFIG_APP) {
SOURCES += \
   main.cpp \
#  main_ffmpeg.cpp \
#  main_v4l2.cpp \
#  main_demux.cpp \
#  main_omxplayer.cpp
}

SOURCES += \
    openmaxiltextureloader.cpp \
    omx_wrapper/Locker.cpp \
    omx_wrapper/Event.cpp \
    omx_wrapper/OMXComponent.cpp \
#    glwidget.cpp \
#    ilclient/ilcore.c \
#    ilclient/ilclient.c \
#    video.cpp \
    omx_wrapper/omxtunnel.cpp \
    omx_imageelement.cpp \
    omx_videosurfaceelement.cpp \
#    omx_texturedelement.cpp \
#    omx_videoprocessor.cpp \     # [1]
    omx_camerasurfaceelement.cpp \
    omx_audioprocessor.cpp \
    omx_mediaprocessor.cpp \
#    omx_videograph.cpp \         # [1]
    omx_wrapper/OMX_Core.cpp \
    omx_mediaprocessorelement.cpp \
    omx_globals.cpp \
    omx_textureprovider.cpp \
    omx_playeraudio.cpp \
    omx_reader.cpp

SOURCES += \
    omxplayer_lib/Srt.cpp \
    omxplayer_lib/Unicode.cpp \
    omxplayer_lib/SubtitleRenderer.cpp \
    omxplayer_lib/OMXVideo.cpp \
    omxplayer_lib/OMXThread.cpp \
    omxplayer_lib/OMXSubtitleTagSami.cpp \
    omxplayer_lib/OMXStreamInfo.cpp \
    omxplayer_lib/OMXReader.cpp \
    omxplayer_lib/OMXPlayerVideo.cpp \
    omxplayer_lib/OMXPlayerSubtitles.cpp \
    omxplayer_lib/OMXPlayerAudio.cpp \
#    omxplayer_lib/omxplayer.cpp \
    omxplayer_lib/OMXOverlayCodecText.cpp \
    omxplayer_lib/OMXCore.cpp \
    omxplayer_lib/OMXClock.cpp \
    omxplayer_lib/OMXAudioCodecOMX.cpp \
    omxplayer_lib/OMXAudio.cpp \
    omxplayer_lib/File.cpp \
    omxplayer_lib/DynamicDll.cpp \
    omxplayer_lib/BitstreamConverter.cpp \
    omxplayer_lib/linux/XMemUtils.cpp \
    omxplayer_lib/linux/RBP.cpp \
    omxplayer_lib/utils/RegExp.cpp \
    omxplayer_lib/utils/PCMRemap.cpp \
    omxplayer_lib/utils/log.cpp

HEADERS  += \
    openmaxiltextureloader.h \
    omx_wrapper/Locker.h \
    omx_wrapper/ILocker.h \
    omx_wrapper/IEvent.h \
    omx_wrapper/Event.h \
    omx_wrapper/OMXComponent.h \
    omx_wrapper/MyDeleter.h \
#    glwidget.h \
    lgl_logging.h \
#    ilclient/ilclient.h \
    omx_wrapper/omxtunnel.h \
    omx_imageelement.h \
    omx_videosurfaceelement.h \
#    omx_texturedelement.h \    # [1]
#    omx_videoprocessor.h \
    omx_camerasurfaceelement.h \
    omx_texture.h \
    omx_qthread.h \
    omx_audioprocessor.h \
    omx_globals.h \
    omx_mediaprocessor.h \
#    omx_videograph.h \         # [1]
    omxplayer_lib/DllOMX.h \
    omxplayer_lib/DllAvFormat.h \
    omxplayer_lib/BitstreamConverter.h \
    omxplayer_lib/OMXSubtitleTagSami.h \
    omxplayer_lib/system.h \
    omx_wrapper/OMX_Core.h \
    omxplayer_lib/DllSwResample.h \
    omxplayer_lib/DllAvUtil.h \
    omxplayer_lib/DllAvFilter.h \
    omxplayer_lib/DllAvCodec.h \
    omx_mediaprocessorelement.h \
    omx_textureprovider.h \
    omx_playeraudio.h \
    omx_reader.h

HEADERS += \
    omxplayer_lib/Unicode.h \
    omxplayer_lib/SubtitleRenderer.h \
    omxplayer_lib/OMXVideo.h \
    omxplayer_lib/OMXThread.h \
    omxplayer_lib/OMXStreamInfo.h \
    omxplayer_lib/OMXReader.h \
    omxplayer_lib/OMXPlayerVideo.h \
    omxplayer_lib/OMXPlayerSubtitles.h \
    omxplayer_lib/OMXPlayerAudio.h \
    omxplayer_lib/OMXCore.h \
    omxplayer_lib/OMXClock.h \
    omxplayer_lib/OMXAudioCodecOMX.h \
    omxplayer_lib/OMXAudio.h \
    omxplayer_lib/File.h \
    omxplayer_lib/DynamicDll.h \
    omxplayer_lib/linux/XMemUtils.h \
    omxplayer_lib/linux/RBP.h \
    omxplayer_lib/utils/RegExp.h \
    omxplayer_lib/utils/PCMRemap.h \
    omxplayer_lib/utils/log.h

OTHER_FILES += \
    main.qml \
    changelog.txt \
    tools/compile_ffmpeg.sh \
    tools/extract_aac_stream.sh \
    tools/extract_h264_stream.sh \
    tools/prepare_openmaxil_backend.sh \
    omxplayer_lib/omxplayer.cpp \
    omxplayer_lib/OMXPLAYER_VERSION

contains(DEFINES, CONFIG_APP) {
RESOURCES += resources.qrc
}
