/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    08.10.2013
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

POC_ControlBar {
    // Layout containing the main buttons.
    RowLayout {
        id:     barMain
        width:  parent.width
        height: parent.height
        opacity: 1.0

        Slider {
            id: sliderPosition
            width:                    parent.width
            Layout.alignment:         Qt.AlignVCenter | Qt.AlignHCenter
            Layout.fillWidth: true
            maximumValue:             1.0
            minimumValue:             0.0
            stepSize:                 0.01
            value: (controlBar.parent.source).position/(controlBar.parent.source).duration;
            updateValueWhileDragging: false

            // NOTE: Remember to avoid seeking on value changed. That will result in
            // seeking for every set of the value prop.

            Keys.onPressed: {
                if (event.key !== Qt.Key_Space)
                    return;
                seek();
                event.accepted = true;
            }

            onPressedChanged: {
                if (pressed !== false)
                    return;
                seek();
            }

            KeyNavigation.right: buttonPlayPause
            KeyNavigation.down:  buttonPlayPause
            KeyNavigation.tab:   buttonPlayPause
            KeyNavigation.left:  buttonVolume
            KeyNavigation.up:    buttonVolume

            function seek() {
                var position = (controlBar.parent.source).position;
                var duration = (controlBar.parent.source).duration;
                console.log("Seeking to " + value*position/duration);
                (controlBar.parent.source).seek(value*duration);
            }
        }

        Button {
            id: buttonPlayPause
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Play/Pause"
            onClicked: (controlBar.parent.source).playPause();

            KeyNavigation.right: buttonStop
            KeyNavigation.down:  buttonStop
            KeyNavigation.tab:   buttonStop
            KeyNavigation.left:  sliderPosition
            KeyNavigation.up:    sliderPosition
        }

        Button {
            id: buttonStop
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Stop"
            onClicked: controlBar.parent.source.stop();

            KeyNavigation.right: buttonVolume
            KeyNavigation.down:  buttonVolume
            KeyNavigation.tab:   buttonVolume
            KeyNavigation.left:  buttonPlayPause
            KeyNavigation.up:    buttonPlayPause
        }

        Button {
            id: buttonVolume
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Volume"
            onClicked: {
                controlBar.state = "VOLUME";
                barVolume.focus  = true;
            }

            KeyNavigation.right: buttonMetaData
            KeyNavigation.down:  buttonMetaData
            KeyNavigation.tab:   buttonMetaData
            KeyNavigation.left:  buttonStop
            KeyNavigation.up:    buttonStop
        }

        Button {
            id: buttonMetaData
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: qsTr("MetaData")
            onClicked: metaData.showAnimated();

            KeyNavigation.right: buttonLegend
            KeyNavigation.down:  buttonLegend
            KeyNavigation.tab:   buttonLegend
            KeyNavigation.left:  buttonVolume
            KeyNavigation.up:    buttonVolume
        }

        Button {
            id: buttonLegend
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: qsTr("Legend")
            onClicked: legend.showAnimated();

            KeyNavigation.right: buttonOpen
            KeyNavigation.down:  buttonOpen
            KeyNavigation.tab:   buttonOpen
            KeyNavigation.left:  buttonMetaData
            KeyNavigation.up:    buttonMetaData
        }

        Button {
            id: buttonOpen
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: qsTr("Open")
            onClicked: fileBrowser.showAnimated();

            KeyNavigation.right: buttonQuit
            KeyNavigation.down:  buttonQuit
            KeyNavigation.tab:   buttonQuit
            KeyNavigation.left:  buttonLegend
            KeyNavigation.up:    buttonLegend
        }

        Button {
            id: buttonQuit
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: qsTr("Quit")
            onClicked: Qt.quit()

            KeyNavigation.right: sliderPosition
            KeyNavigation.down:  sliderPosition
            KeyNavigation.tab:   sliderPosition
            KeyNavigation.left:  buttonOpen
            KeyNavigation.up:    buttonOpen
        }

        // If esc if pressed, focus to the parent.
        Keys.onPressed: {
            if (event.key === Qt.Key_Escape)
                hideAnimated();
        }
    }

    // Layout containing the volume bar.
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
            id: sliderVolume
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

        // Handle the escape key to exit volume "mode".
        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                controlBar.state = "MAIN";
                barMain.focus    = true;
                event.accepted   = true;
            }
        }
    }

    // When focused pass the focus to the currently visible layout.
    onFocusChanged: {
        if (focus) {
            // Exiting from the main bar means dismiss the control bar.
            controlBarEnabled();
            if (state == "MAIN") {
                console.log("Giving focus to the main bar...");
                barMain.focus = true;
            }
            else if (state == "VOLUME") {
                console.log("Giving focus to the volume bar...");
                barVolume.focus = true;
            }
        }
    }

    /**
      * Give focus to the first component.
      */
    function acquireFocus() {
        if (state == "MAIN")
            sliderPosition.focus = true;
        else if (state == "VOLUME")
            sliderVolume.focus = true;
        else
            console.log("Unknown state!");
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

            NumberAnimation {
                target:      barMain
                property:    "opacity"
                duration:    1000
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                target:      barVolume
                property:    "opacity"
                duration:    1000
                easing.type: Easing.InOutQuad
            }
        },

        Transition {
            from: "VOLUME"
            to:   "MAIN"

            NumberAnimation {
                target:      barMain
                property:    "opacity"
                duration:    1000
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                target:      barVolume
                property:    "opacity"
                duration:    1000
                easing.type: Easing.InOutQuad
            }
        }
    ]
}
