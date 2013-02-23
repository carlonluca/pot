#
# Project: PiOmxTextures
# Author:  Luca Carlon
# Date:    12.03.2012
#

QT += core core-private gui gui-private opengl quick quick-private

TARGET   = PiOmxTextures
TEMPLATE = app

# External
LIBS += -lopenmaxil -lGLESv2 -lEGL -lbcm_host -lvcos -lrt -lv4l2
LIBS += -lavformat -lavcodec -lavutil
# Internal
# NOTE: I had issues with versions compiled from recent sources.
#LIBS += -L$$_PRO_FILE_PWD_/3rdparty/lib -lavformat -lavcodec -lavutil
# For omxplayer.
LIBS += -lfreetype -lWFC -lpcre

INCLUDEPATH += \
   omx_wrapper \
   ilclient \
   3rdparty/include
INCLUDEPATH += \
   omxplayer_lib \
   omxplayer_lib/utils \
   omxplayer_lib/linux

VERSION = 4.0

# Flags used bu hello_pi examples:
#-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
# -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE
# -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX
# -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST
# -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi

DEFINES += __STDC_CONSTANT_MACROS \
   __STDC_LIMIT_MACROS \
   TARGET_POSIX \
   _LINUX \
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
   HAVE_OMXLIB \
   STANDALONE

#DEFINES += LOG_LEVEL_DEBUG
DEFINES += VERBOSE
DEFINES += ENABLE_VIDEO_TEST
DEFINES += ENABLE_MEDIA_PROCESSOR

QMAKE_CXXFLAGS_DEBUG += -rdynamic
# For omxplayer.
QMAKE_CXXFLAGS += -std=c++0x -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DHAVE_CMAKE_CONFIG -D__VIDEOCORE4__ -U_FORTIFY_SOURCE -Wall -DUSE_EXTERNAL_FFMPEG  -DHAVE_LIBAVCODEC_AVCODEC_H -DHAVE_LIBAVUTIL_OPT_H -DHAVE_LIBAVUTIL_MEM_H -DHAVE_LIBAVUTIL_AVUTIL_H -DHAVE_LIBAVFORMAT_AVFORMAT_H -DHAVE_LIBAVFILTER_AVFILTER_H -DOMX -DOMX_SKIP64BIT -ftree-vectorize -DUSE_EXTERNAL_OMX -DTARGET_RASPBERRY_PI -DUSE_EXTERNAL_LIBBCM_HOST

SOURCES += \
    main.cpp \
#    main_ffmpeg.cpp \
#    main_v4l2.cpp \
#    main_demux.cpp \
#    main_omxplayer.cpp \
    openmaxiltextureloader.cpp \
    omx_wrapper/Locker.cpp \
    omx_wrapper/Event.cpp \
    omx_wrapper/OMXComponent.cpp \
    glwidget.cpp \
    ilclient/ilcore.c \
    ilclient/ilclient.c \
#    video.cpp \
    omx_wrapper/omxtunnel.cpp \
    omx_imageelement.cpp \
    omx_videosurfaceelement.cpp \
    omx_texturedelement.cpp \
    omx_videoprocessor.cpp \
    omx_camerasurfaceelement.cpp \
    omx_textureproviderqquickitem.cpp \
    omx_audioprocessor.cpp \
    omx_mediaprocessor.cpp \
    omx_videograph.cpp \
    omx_wrapper/OMX_Core.cpp \
    omx_mediaprocessorelement.cpp

SOURCES += \
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
    omxplayer_lib/omxplayer.cpp \
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
    glwidget.h \
    lgl_logging.h \
    ilclient/ilclient.h \
    omx_wrapper/omxtunnel.h \
    omx_imageelement.h \
    omx_videosurfaceelement.h \
    omx_texturedelement.h \
    omx_videoprocessor.h \
    omx_camerasurfaceelement.h \
    omx_textureproviderqquickitem.h \
    omx_texture.h \
    omx_qthread.h \
    omx_audioprocessor.h \
    omx_globals.h \
    omx_mediaprocessor.h \
    omx_videograph.h \
    omxplayer_lib/DllOMX.h \
    omxplayer_lib/DllAvFormat.h \
    omxplayer_lib/BitstreamConverter.h \
    omxplayer_lib/OMXSubtitleTagSami.h \
    omx_wrapper/OMX_Core.h \
    omxplayer_lib/ScopeExit.h \
    omxplayer_lib/Enforce.h \
    omxplayer_lib/DllSwResample.h \
    omxplayer_lib/DllAvUtil.h \
    omxplayer_lib/DllAvFilter.h \
    omxplayer_lib/DllAvCodec.h \
    omx_mediaprocessorelement.h

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
    tools/extract_h264_stream.sh

RESOURCES += \
    resources.qrc
