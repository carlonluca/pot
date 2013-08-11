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
    signal nextImage
    signal prevImage
    signal rotateCounter
    signal rotateClock

    // Layout containing the main buttons.
    RowLayout {
        id:     barMain
        width:  parent.width
        height: parent.height
        opacity: 1.0

        Button {
            id: buttonPrevImage
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Previous Image"
            onClicked: prevImage()

            KeyNavigation.right: buttonNextImage
            KeyNavigation.down:  buttonNextImage
            KeyNavigation.tab:   buttonNextImage
            KeyNavigation.left:  buttonNextImage
            KeyNavigation.up:    buttonNextImage
        }

        Button {
            id: buttonNextImage
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Next Image"
            onClicked: nextImage()

            KeyNavigation.right: buttonPrevImage
            KeyNavigation.down:  buttonPrevImage
            KeyNavigation.tab:   buttonPrevImage
            KeyNavigation.left:  buttonPrevImage
            KeyNavigation.up:    buttonPrevImage
        }

        Button {
            id: buttonRotateCounter
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Rotate Counter"
            onClicked: rotateCounter()

            KeyNavigation.right: buttonPrevImage
            KeyNavigation.down:  buttonPrevImage
            KeyNavigation.tab:   buttonPrevImage
            KeyNavigation.left:  buttonPrevImage
            KeyNavigation.up:    buttonPrevImage
        }

        Button {
            id: buttonRotateClock
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: "Rotate Clock"
            onClicked: rotateClock()

            KeyNavigation.right: buttonPrevImage
            KeyNavigation.down:  buttonPrevImage
            KeyNavigation.tab:   buttonPrevImage
            KeyNavigation.left:  buttonPrevImage
            KeyNavigation.up:    buttonPrevImage
        }

        // Focus to prev image.
        onFocusChanged: {
            if (activeFocus)
                buttonPrevImage.focus = true;
        }

        // If esc if pressed, focus to the parent.
        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                // TODO: Bring this back into the signal handler.
                controlBar.parent.parent.parent.focus = true;
                controlBarDismissed();
            }
        }
    }

    // When focused pass the focus to the currently visible layout.
    onFocusChanged: {
        if (focus) {
            // Exiting from the main bar means dismiss the control bar.
            controlBarEnabled();
            console.log("Giving focus to the main bar...");
            barMain.focus = true;
        }
    }
}
