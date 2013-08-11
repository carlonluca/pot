/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    08.11.2013
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
import "POC_Constants.js" as POC_Constants

Image {
    property var orientation: POC_Constants.Orientation.HORIZONTAL

    // The usage of this value instead of simply the rotation property is due to the
    // fact that rotation assumes all the intermediate values. If a new rotation is
    // requested before the current is completed, I would not know the new value to
    // give to the rotation. I also cannot use the orientation property because I need
    // to provide a complete angle, not simply the value in [0, 360].
    property int currentRotationValue: 0

    id: imageComponent
    anchors.centerIn: parent

    width:  parent.width
    height: parent.height

    fillMode: Image.PreserveAspectFit

    onCurrentRotationValueChanged: rotation = currentRotationValue

    Behavior on rotation {
        NumberAnimation {
            duration: 300
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on width {
        NumberAnimation {
            duration: 300
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on height {
        NumberAnimation {
            duration: 300
            easing.type: Easing.InOutQuad
        }
    }

    // Always animate opacity.
    Behavior on opacity {
        NumberAnimation {
            duration: 600
            easing.type: Easing.OutQuad
        }
    }

    /**
      * Rotates the image clockwise.
      */
    function rotateClock() {
        currentRotationValue += 90;

        switch (orientation) {
        case POC_Constants.Orientation.HORIZONTAL_UPDOWN:
            orientation = POC_Constants.Orientation.VERTICAL_UPDOWN;
            break;
        case POC_Constants.Orientation.HORIZONTAL:
            orientation = POC_Constants.Orientation.VERTICAL;
            break;
        case POC_Constants.Orientation.VERTICAL:
            orientation = POC_Constants.Orientation.HORIZONTAL_UPDOWN;
            break;
        case POC_Constants.Orientation.VERTICAL_UPDOWN:
            orientation = POC_Constants.Orientation.HORIZONTAL;
            break;
        default:
            break;
        }

        fitIntoParent();
    }

    /**
      * Rotates the image counter-clockwise.
      */
    function rotateCounter() {
        currentRotationValue -= 90;

        switch (orientation) {
        case POC_Constants.Orientation.HORIZONTAL_UPDOWN:
            orientation = POC_Constants.Orientation.VERTICAL;
            break;
        case POC_Constants.Orientation.HORIZONTAL:
            orientation = POC_Constants.Orientation.VERTICAL_UPDOWN;
            break;
        case POC_Constants.Orientation.VERTICAL:
            orientation = POC_Constants.Orientation.HORIZONTAL;
            break;
        case POC_Constants.Orientation.VERTICAL_UPDOWN:
            orientation = POC_Constants.Orientation.HORIZONTAL_UPDOWN;
            break;
        default:
            break;
        }

        fitIntoParent();
    }

    /**
      * Makes the view fit into the parent view according to the current orientation.
      */
    function fitIntoParent() {
        switch (orientation) {
        case POC_Constants.Orientation.HORIZONTAL_UPDOWN:
        case POC_Constants.Orientation.HORIZONTAL:
            imageComponent.width  = parent.width;
            imageComponent.height = parent.height;
            break;
        case POC_Constants.Orientation.VERTICAL:
        case POC_Constants.Orientation.VERTICAL_UPDOWN:
            imageComponent.width  = parent.height;
            imageComponent.height = parent.width;
            break;
        default:
            break;
        }
    }
}
