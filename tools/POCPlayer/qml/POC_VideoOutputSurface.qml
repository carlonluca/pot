import QtQuick 2.0
import QtMultimedia 5.0

import "POC_Constants.js" as POC_Constants

VideoOutput {
    property var orientation: POC_Constants.Orientation.HORIZONTAL

    anchors.centerIn: parent
    fillMode: VideoOutput.PreserveAspectFit

    width:  parent.width
    height: parent.height

    // The usage of this value instead of simply the rotation property is due to the
    // fact that rotation assumes all the intermediate values. If a new rotation is
    // requested before the current is completed, I would not know the new value to
    // give to the rotation. I also cannot use the orientation property because I need
    // to provide a complete angle, not simply the value in [0, 360].
    property int currentRotationValue: 0

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
            width  = parent.width;
            height = parent.height;
            break;
        case POC_Constants.Orientation.VERTICAL:
        case POC_Constants.Orientation.VERTICAL_UPDOWN:
            width  = parent.height;
            height = parent.width;
            break;
        default:
            break;
        }
    }

    // Text containing the position/duration.
    POC_TextPosition {
        anchors.top:   parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
