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
import QtQuick.Layouts 1.0

Rectangle {
    color:   "black"
    opacity: 0.4
    radius:  5

    // The content.
    ColumnLayout {
        width:  parent.width
        height: parent.height*5/6
        anchors.centerIn: parent

        POC_LegendItem {
            text:  qsTr("Media Player - Stop [s]")
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }

        POC_LegendItem {
            text:  qsTr("Media Player - Play/Pause [p]")
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }

        POC_LegendItem {
            text:  qsTr("Media Player - Volume Up [+]")
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }

        POC_LegendItem {
            text:  qsTr("Media Player - Volume Down [-]")
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }

        POC_LegendItem {
            text:  qsTr("Activate Toolbar [Down Arrow]")
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }

        POC_LegendItem {
            text:  qsTr("Show/Hide Legend [l]")
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }
    }

    // Always animate opacity.
    Behavior on opacity {
        NumberAnimation {
            duration: 1000
            easing.type: Easing.OutQuad
        }
    }

    // Convenience functions.
    function toggleVisibility() {
        if (opacity > 0)
            hideAnimated();
        else
            showAnimated();
    }

    function showAnimated() {
        opacity = 0.5;
    }

    function hideAnimated() {
        opacity = 0.0;
    }
}
