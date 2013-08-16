/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.17.2013
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
import QtMultimedia 5.0

// The unique video surface.
VideoOutput {
    signal controlBarDismissed()

    height: parent.height
    width:  parent.width

    fillMode: VideoOutput.Stretch

    // Used to show the control bar.
    MouseArea {
        anchors.fill: parent
        onClicked:
            controlBar.toggleAnimated();

        z: 1
    }

    // The control bar.
    POC_ControlBarVideo {
        id: controlBar

        z: 2

        onControlBarDismissed: parent.controlBarDismissed()
    }

    /**
      * Shows and give focus to the control bar.
      */
    function showControlBar() {
        controlBar.showAnimated();
    }

    /**
      * Should "go on" inside the media or to the next media.
      */
    function goOnMedia() {
        // Unimplemented.
    }

    /**
      * Should "go back" inside the media or to the prev media.
      */
    function goBackMedia() {
        // Unimplemented.
    }

    // Text containing the position/duration.
    POC_TextPosition {
        anchors.top:   parent.top
        anchors.right: parent.right
    }

    // Automatically pass the focus to the control bar.
    onFocusChanged: {
        if (focus) {
            console.log("Giving focus to the video control bar...");
            controlBar.focus = true;
        }
    }
}
