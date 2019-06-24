#!/bin/bash

#
# Project: PiOmxTexturesVideoLayer
# Author:  Luca Carlon
# Date:    02.14.2016
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

while pgrep $1 > /dev/null; do sleep 1; done
setterm -foreground white -clear all > /dev/tty0
