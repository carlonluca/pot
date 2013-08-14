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

Item {
    property bool isHidden: true

    signal controlBarDismissed
    signal controlBarEnabled

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

    // Methods to show and hide.
    function toggleAnimated() {
        if (!isHidden)
            hideAnimated();
        else
            showAnimated();
    }

    function showAnimated() {
        animationShow.running = true;
        acquireFocus();
        isHidden = false;
        controlBarEnabled();
    }

    function hideAnimated() {
        animationHide.running = true;
        isHidden = true;
        controlBarDismissed();
    }
}
