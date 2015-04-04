#
# Project: PiOmxTextures
# Author:  Luca Carlon
# Date:    04.03.2015
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

TEMPLATE = app

QT += core core-private gui gui-private opengl quick quick-private
CONFIG += no_private_qt_headers_warning

# Macro definitions
#DEFINES += LOG_LEVEL_DEBUG
DEFINES += VERBOSE
DEFINES += ENABLE_VIDEO_TEST
DEFINES += ENABLE_MEDIA_PROCESSOR

include(piomxtextures_app.pri)
include($$_PRO_FILE_PWD_/../piomxtextures_src/piomxtextures_src.pri)

SOURCES += \
   main.cpp \
#  main_ffmpeg.cpp \
#  main_v4l2.cpp \
#  main_demux.cpp \
#  main_omxplayer.cpp

RESOURCES += resources.qrc
