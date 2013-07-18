/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.14.2013
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
import QtQuick.Controls 1.0

Item {
    id:     controlBar
    width:  parent.width
    height: parent.height*1/5
    state:  "MAIN"

    // Just the background.
    Rectangle {
        width:  parent.width
        height: parent.height
        color:  "dark gray"
        opacity: 0.5
    }

    // Layout containing the buttons.
    RowLayout {
        id:     barMain
        width:  parent.width
        height: parent.height
        Layout.column: 3
        opacity: 1.0

        Button {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Pause/Pause"
            onClicked: (controlBar.parent.source).playPause();
        }

        Button {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Stop"
            onClicked: controlBar.parent.source.stop();
        }

        Button {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Volume"
            onClicked: controlBar.state = "VOLUME"
        }
    }

    // Layout containing the volume.
    RowLayout {
        id:     barVolume
        width:  parent.width
        height: parent.height
        Layout.column: 3

        Text {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: qsTr("Volume Up")
        }

        Slider {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            maximumValue:     1.0
            minimumValue:     0.0
            stepSize:         0.05
            onValueChanged:   controlBar.parent.source.volume = value;
            value:            controlBar.parent.source.volume
        }

        Text {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: qsTr("Volume Down")
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape)
                controlBar.state = "MAIN";
        }
    }

    // The states.
    states: [
        State {
            name: "MAIN"
            PropertyChanges {
                target:  barMain
                opacity: 1.0
            }
            PropertyChanges {
                target:  barVolume
                opacity: 0.0
            }
        },

        State {
            name: "VOLUME"
            PropertyChanges {
                target:  barMain
                opacity: 0.0
            }
            PropertyChanges {
                target:  barVolume
                opacity: 1.0
            }
        }
    ]

    // Transitions.
    transitions: [
        Transition {
            from: "MAIN"
            to:   "VOLUME"
            NumberAnimation {target: barMain; property: "opacity"; duration: 1000; easing.type: Easing.InOutQuad}
            NumberAnimation {target: barVolume; property: "opacity"; duration: 1000; easing.type: Easing.InOutQuad}
        },

        Transition {
            from: "VOLUME"
            to:   "MAIN"
            NumberAnimation {target: barMain; property: "opacity"; duration: 1000; easing.type: Easing.InOutQuad}
            NumberAnimation {target: barVolume; property: "opacity"; duration: 1000; easing.type: Easing.InOutQuad}
        }
    ]
}
