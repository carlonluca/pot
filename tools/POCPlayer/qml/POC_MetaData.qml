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

/**
  Componenet to show metadata.
  */
POC_AnimatedOverlay {
    property var source: null

    Text {
        anchors.fill: parent
        color: "white"
        font.pixelSize: 30
        text: "METADATA\n" +
              "Title: " + formatString(source.metaData.title) + "\n" +
              "Subtitle: " + formatString(source.metaData.subTitle) + "\n" +
              "Artist: " + formatString(source.metaData.author) + "\n" +
              "Date of the Media: " + formatString(source.metaData.date) + "\n" +
              "Album: " + formatString(source.metaData.albumTitle) + "\n" +
              "Album Artist: " + formatString(source.metaData.albumArtist);
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment:   Text.AlignVCenter

        function formatString(s) {
            if (s === null || s === undefined)
                return "-";
            return s;
        }
    }
}
