#
# Project: PiOmxTexturesVideoLayer
# Author:  Luca Carlon
# Date:    01.10.2016
#
# Copyright (c) 2016 Luca Carlon. All rights reserved.
#
# This file is part of PiOmxTexturesVideoLayer.
#
# PiOmxTexturesVideoLayer is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# PiOmxTexturesVideoLayer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with PiOmxTexturesVideoLayer. If not, see <http://www.gnu.org/licenses/>.
#

TEMPLATE = lib

VERSION = 0.6.5
DEFINES += VERSION=\\\"$$VERSION\\\"

QT += quick qml dbus gui multimedia
CONFIG += qt plugin
DESTDIR = imports/VideoLayer

QMAKE_LFLAGS += -rdynamic

INCLUDEPATH += \
	$$_PRO_FILE_PWD_/../3rdparty/LightLogger \
	$$_PRO_FILE_PWD_/../3rdparty/LightSmartPtr \
        $$_PRO_FILE_PWD_/../piomxtextures_src \
        $$[QT_SYSROOT]/opt/vc/include

HEADERS += \
	omx_audio.h \
	omx_logging_cat.h \
	omx_video.h \
	omx_videolayer.h \
	omx_piomxtexturesplugin.h \
	omx_omxplayercontroller.h

SOURCES += \
	omx_audio.cpp \
	omx_video.cpp \
	omx_videolayer.cpp \
	omx_piomxtexturesplugin.cpp \
	omx_omxplayercontroller.cpp

RESOURCES += \
	resources.qrc

OTHER_FILES += \
	samples/test.qml \
	samples/test_animation.qml
