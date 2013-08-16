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

Item {
    signal controlBarDismissed()

    property var currentSurface: outputSurface_1
    property bool isControlBarVisible: false

    width:  parent.width
    height: parent.height

    // Used to show the control bar.
    MouseArea {
        anchors.fill: parent
        onClicked: controlBar.toggleAnimated()

        z: 1
    }

    // This component cannot be the parent of all the others because
    // rotation would result in a rotation of all the contained components.
    POC_ImageOutputSurface {
        id: outputSurface_1
        opacity: 1.0

        z: 0
    }

    POC_ImageOutputSurface {
        id: outputSurface_2
        opacity: 0.0

        z: 0
    }

    // The control bar.
    POC_ControlBarImage {
        id: controlBar

        // Handle the messages.
        onNextImage:     goOnMedia()
        onPrevImage:     goBackMedia()
        onRotateClock:   currentSurface.rotateClock();
        onRotateCounter: currentSurface.rotateCounter();

        z: 2

        onControlBarDismissed: parent.controlBarDismissed()
    }

    /**
      * Loads the image into the currently visible surface.
      */
    function showImage(fileUri) {
        currentSurface.source = fileUri;
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
        var nextSurface;
        if (currentSurface == outputSurface_1)
            nextSurface = outputSurface_2;
        else
            nextSurface = outputSurface_1;

        nextSurface.source = "file://" + utils.getNextImage(utils.getPathFromUri(currentSurface.source));
        switchSurfaces(nextSurface);
    }

    /**
      * Should "go back" inside the media or to the prev media.
      */
    function goBackMedia() {
        var nextSurface;
        if (currentSurface == outputSurface_1)
            nextSurface = outputSurface_2;
        else
            nextSurface = outputSurface_1;

        nextSurface.source = "file://" + utils.getPrevImage(utils.getPathFromUri(currentSurface.source));
        switchSurfaces(nextSurface);
    }

    /**
      * Switches the visible surface.
      */
    function switchSurfaces(nextSurface) {
        currentSurface.opacity = 0.0;
        nextSurface.opacity    = 1.0;
        currentSurface = nextSurface;
    }
}
