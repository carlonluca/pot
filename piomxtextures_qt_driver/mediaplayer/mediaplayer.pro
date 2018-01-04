#
# Project: PiOmxTextures
# Author:  Luca Carlon
# Date:    04.06.2013
#
# Copyright (c) 2012-2015 Luca Carlon. All rights reserved.
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
TEMPLATE = lib

QT += quick network

PLUGIN_TYPE = mediaservice
PLUGIN_CLASS_NAME = OpenMAXILPlayerServicePlugin

#QT     += multimedia-private network
#CONFIG += no_private_qt_headers_warning

#qtHaveModule(widgets) {
#    QT += widgets multimediawidgets-private
#    DEFINES += HAVE_WIDGETS
#}

#config_xvideo:qtHaveModule(widgets) {
#    DEFINES += HAVE_XVIDEO
#    LIBS += -lXv -lX11 -lXext
#}

include($$_PRO_FILE_PWD_/../../piomxtextures_src/piomxtextures_src.pri)

LIBS += -lbrcmEGL

HEADERS += \
    $$PWD/openmaxilplayercontrol.h \
    $$PWD/openmaxilplayerservice.h \
    $$PWD/openmaxilstreamscontrol.h \
    $$PWD/openmaxilmetadataprovider.h \
    $$PWD/openmaxilavailabilitycontrol.h \
    $$PWD/openmaxilplayerserviceplugin.h \
    $$PWD/openmaxilvideorenderercontrol.h \
    $$PWD/openmaxilvideoprobe.h

SOURCES += \
    $$PWD/openmaxilplayercontrol.cpp \
    $$PWD/openmaxilplayerservice.cpp \
    $$PWD/openmaxilstreamscontrol.cpp \
    $$PWD/openmaxilmetadataprovider.cpp \
    $$PWD/openmaxilavailabilitycontrol.cpp \
    $$PWD/openmaxilplayerserviceplugin.cpp \
    $$PWD/openmaxilvideorenderercontrol.cpp \
    $$PWD/openmaxilvideoprobe.cpp

OTHER_FILES += mediaplayer.json
