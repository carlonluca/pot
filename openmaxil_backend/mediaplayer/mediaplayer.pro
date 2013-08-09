#
# Project: PiOmxTextures
# Author:  Luca Carlon
# Date:    04.06.2013
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

TARGET = openmaxilmediaplayer

PLUGIN_TYPE = mediaservice
PLUGIN_CLASS_NAME = OpenMAXILPlayerServicePlugin
load(qt_plugin)

QT += quick

include(../common.pri)

message($$_PRO_FILE_PWD_)
INCLUDEPATH += $$PWD
INCLUDEPATH += \
#   $$_PRO_FILE_PWD_/../3rdparty/ffmpeg/include \
   $$_PRO_FILE_PWD_/../3rdparty/PiOmxTextures/include
LIBS += \
   -lEGL \
#   -L$$_PRO_FILE_PWD_/../3rdparty/ffmpeg/lib -lavformat -lavcodec -lavutil -lswscale -lswresample \
   -L$$_PRO_FILE_PWD_/../3rdparty/PiOmxTextures/lib -lPiOmxTextures

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
