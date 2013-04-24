#
# Project: openmaxil QtMultimedia plugin
# Author:  Luca Carlon (carlon.luca (AT) gmail.com)
# Date:    04.06.2013
#

TARGET = openmaxilmediaplayer

PLUGIN_TYPE = mediaservice
PLUGIN_CLASS_NAME = OpenMAXILPlayerServicePlugin
load(qt_plugin)

include(../common.pri)

message($$_PRO_FILE_PWD_)
INCLUDEPATH += $$PWD
INCLUDEPATH += \
   $$_PRO_FILE_PWD_/../3rdparty/ffmpeg/include \
   $$_PRO_FILE_PWD_/../3rdparty/PiOmxTextures/include
LIBS += \
   -lEGL \
   -L$$_PRO_FILE_PWD_/../3rdparty/ffmpeg/lib -lavformat -lavcodec -lavutil -lswscale -lswresample \
   -L$$_PRO_FILE_PWD_/../3rdparty/PiOmxTextures/lib -lPiOmxTextures
QMAKE_LFLAGS += -Wl,-unresolved-symbols=ignore-all
QMAKE_LFLAGS -= -Wl,--no-undefined

HEADERS += \
    $$PWD/openmaxilplayercontrol.h \
    $$PWD/openmaxilplayerservice.h \
    $$PWD/openmaxilstreamscontrol.h \
    $$PWD/openmaxilmetadataprovider.h \
    $$PWD/openmaxilavailabilitycontrol.h \
    $$PWD/openmaxilplayerserviceplugin.h \
    $$PWD/openmaxilvideorenderercontrol.h

SOURCES += \
    $$PWD/openmaxilplayercontrol.cpp \
    $$PWD/openmaxilplayerservice.cpp \
    $$PWD/openmaxilstreamscontrol.cpp \
    $$PWD/openmaxilmetadataprovider.cpp \
    $$PWD/openmaxilavailabilitycontrol.cpp \
    $$PWD/openmaxilplayerserviceplugin.cpp \
    $$PWD/openmaxilvideorenderercontrol.cpp

OTHER_FILES += \
    mediaplayer.json
