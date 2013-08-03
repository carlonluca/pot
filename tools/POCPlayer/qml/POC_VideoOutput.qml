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
    height: parent.height
    width:  parent.width

    fillMode: VideoOutput.Stretch

    // The control bar.
    POC_ControlBar {
        signal controlBarDismissed
        signal controlBarEnabled

        id:             controlBar
        x:              parent.width
        anchors.bottom: parent.bottom

        PropertyAnimation {
            id: animationShow
            easing.type: Easing.OutElastic
            duration: 1000
            target: controlBar; property: "x"; to: 0
        }

        PropertyAnimation {
            id: animationHide
            easing.type: Easing.OutElastic
            duration: 1000
            target: controlBar; property: "x"; to: parent.width
        }

        onControlBarEnabled: {
            showAnimated();
        }

        onControlBarDismissed: {
            hideAnimated();
        }

        // Methods to show and hide.
        function toggleAnimated() {
            if (x === 0)
                hideAnimated();
            else
                showAnimated();
        }

        function showAnimated() {
            animationShow.running = true;
        }

        function hideAnimated() {
            animationHide.running = true;
        }
    }

    // Text containing the position/duration.
    POC_TextPosition {
        anchors.top:   parent.top
        anchors.right: parent.right
    }

    // Automatically pass the focus to the control bar.
    onFocusChanged: {
        if (activeFocus)
            controlBar.focus = true;
    }
}
