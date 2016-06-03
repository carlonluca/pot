#
# Project: PiOmxTexturesVideoLayer
# Author:  Luca Carlon
# Date:    01.10.2016
#
# Copyright (c) 2016 Luca Carlon. All rights reserved.
#
# This file is part of PiOmxTexturesVideoLayer.
#
# PiOmxTexturesQmlUtils is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# PiOmxTexturesQmlUtils is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with PiOmxTexturesQmlUtils. If not, see <http://www.gnu.org/licenses/>.
#

TEMPLATE = lib

VERSION = 0.1.0
DEFINES += VERSION=\\\"$$VERSION\\\"

QT += quick qml multimedia
CONFIG += qt plugin
CONFIG += -std=c++11

INCLUDEPATH += \
	$$_PRO_FILE_PWD_/../3rdparty/LightLogger \
	$$_PRO_FILE_PWD_/../3rdparty/LightSmartPtr \
	$$_PRO_FILE_PWD_/../piomxtextures_src

HEADERS += \
	omx_piomxtexturesplugin.h \
	pot_videoprobe.h

SOURCES += \
	omx_piomxtexturesplugin.cpp \
	pot_videoprobe.cpp

RESOURCES += resources.qrc
