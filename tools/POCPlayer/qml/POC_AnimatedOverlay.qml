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

/**
  Used to show information in overlay with uniform appearance.
  */
Rectangle {
    signal focusRelinquished()

    color:   "black"
    radius:  5
    x:       parent.width*1/10
    y:       parent.height*1/10
    width:   parent.width*5/6
    height:  parent.height*4/6
    opacity: 0.0
    enabled: false

    // Catch any mouse input.
    MouseArea {
        id: mouseCatcher
        x: 0
        y: 0
        width: mainView.width
        height: mainView.height

        onPressed: hideAnimated();
    }

    // Always animate opacity.
    Behavior on opacity {
        NumberAnimation {
            duration: 600
            easing.type: Easing.OutQuad
        }
    }

    // Disappear on any input.
    Keys.onPressed: {
        hideAnimated();
        event.accepted = true;
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
        enabled = true;
        focus = true;
    }

    function hideAnimated() {
        opacity = 0.0;
        focus = false;
        enabled = false;
        focusRelinquished();
    }
}
