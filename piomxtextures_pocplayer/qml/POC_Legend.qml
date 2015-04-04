/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.24.2013
*
* Copyright (c) 2012, 2013 Luca Carlon. All rights reserved.
*
* This file is part of PiOmxTextures.
*
* PiOmxTextures is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* PiOmxTextures is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.0

POC_AnimatedOverlay {
    Text {
        anchors.fill: parent
        color: "white"
        font.pixelSize: 30
        text:  "LEGEND\n" +
               "Media Player - Stop [s]\n" +
               "Media Player - Play/Pause [p]\n" +
               "Media Player - Volume Up [+]\n" +
               "Media Player - Volume Down [-]\n" +
               "Activate Toolbar [Down Arrow]\n" +
               "Show/Hide Legend [l]\n" +
               "Show/Hide MetaData [t]";
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment:   Text.AlignVCenter
    }
}
