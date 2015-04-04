/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    27.05.2014
*
* Copyright (c) 2012, 2013, 2014 Luca Carlon. All rights reserved.
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

Rectangle {
    property bool enableCrossfade: true
    property bool enableVideoFullscreen: true
    property bool enableImageFullscreen: true
    property bool enableFlip: true
    property string resSuffix: "1080p"
    property int animDuration: 5000

    property var transformations: []
    property int index: -1;

    Component.onCompleted: {
        var index = 0;

        if (enableFlip) {
            transformations[index++] = flip;
            transformations[index++] = flip;
        }

        if (enableCrossfade)
            transformations[index++] = crossfade;

        if (enableImageFullscreen) {
            transformations[index++] = doVideoFullscreen;

            if (enableCrossfade)
                transformations[index++] = crossfade;

            transformations[index++] = doVideoFrame;
        }

        if (enableVideoFullscreen) {
            transformations[index++] = doImageFullscreen;

            if (enableCrossfade)
                transformations[index++] = crossfade;

            transformations[index++] = doImageFrame;
        }
    }

    color: "red"

    Timer {
        interval: 7000
        repeat: true
        Component.onCompleted: start();

        onTriggered: {
            index = index + 1;
            index = index % (transformations.length);
            transformations[index]();
        }
    }

    Flipable {
        id: flipable
        property bool flipped: false

        x: 5
        y: 5
        width: parent.width/3*2 - 5*2
        height: parent.height - 5*2

        rotation: 0

        front: Video {
            id: videoOutput
            objectName: "mediaOutput"
            anchors.fill: parent

            Timer {
                id: deferredPlay
                repeat: false
                interval: 5000
                onTriggered: {
                    var tmpSource = videoOutput.source;
                    videoOutput.source = "";
                    videoOutput.source = tmpSource;
                }
            }

            onStopped: {
                console.log("Video stopped.");
                stop();
                seek(0);
                deferredPlay.start();
            }

            function showLocalMedia(localAbsPath) {
                source = "file://" + localAbsPath;
                play();
            }
        }

        back: Rectangle {
            color: "orange"
            anchors.fill: parent

            Text {
                anchors.fill: parent
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment:  Qt.AlignVCenter
                text: qsTr("Turn for the cool side!");
            }
        }

        function toFullscreen() {
            x = 0;
            y = 0;
            width = parent.width;
            height = parent.height;
        }

        function toFrame() {
            x = 5;
            y = 5;
            width = parent.width/3*2 - 5*2;
            height = parent.height - 5*2;
        }

        Behavior on width {NumberAnimation {duration: animDuration; easing.type: Easing.OutBounce}}
        Behavior on height {NumberAnimation {duration: animDuration; easing.type: Easing.OutBounce}}
        Behavior on x {NumberAnimation {duration: animDuration; easing.type: Easing.OutBounce}}
        Behavior on y {NumberAnimation {duration: animDuration; easing.type: Easing.OutBounce}}

        transform: Rotation {
                id: rotation
                origin.x: flipable.width/2
                origin.y: flipable.height/2
                axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
                angle: 0    // the default angle
            }

            states: [
                State {
                    name: "back"
                    PropertyChanges {target: rotation; angle: 180}
                    when: flipable.flipped
                },
                State {
                    name: "front"
                    PropertyChanges {target: rotation; angle: 0}
                    when: !flipable.flipped
                }
            ]

            transitions: Transition {
                NumberAnimation {
                    target: rotation;
                    property: "angle";
                    duration: animDuration
                    easing.type: Easing.OutBounce
                }
            }
    }

    POC_CrossImage {
        id: crossImage

        source1: "qrc:///img/1_" + resSuffix + ".jpg"
        source2: "qrc:///img/2_" + resSuffix + ".jpg"

        width: parent.width/3 - 5*2
        height: parent.height - 5*2
        x: parent.width - 5 - parent.width/3 + 5*2
        y: 5

        animDuration: animDuration

        function toFullscreen() {
            x = 0;
            y = 0;
            width = parent.width;
            height = parent.height;
        }

        function toFrame() {
            width = parent.width/3 - 5*2;
            height = parent.height - 5*2;
            x = parent.width - 5 - parent.width/3 + 5*2;
            y = 5;
        }

        Behavior on width {NumberAnimation {duration: animDuration; easing.type: Easing.OutBounce}}
        Behavior on height {NumberAnimation {duration: animDuration; easing.type: Easing.OutBounce}}
        Behavior on x {NumberAnimation {duration: animDuration; easing.type: Easing.OutBounce}}
        Behavior on y {NumberAnimation {duration: animDuration; easing.type: Easing.OutBounce}}
    }

    Text {
        id: res
        text: qsTr("Resolution: " + resSuffix + ", Video: " + videoOutput.source)
    }

    function crossfade() {
        console.log("Crossfading...");
        crossImage.next();
    }

    function doVideoFullscreen() {
        console.log("Video to fullscreen...");
        flipable.toFullscreen();
    }

    function doVideoFrame() {
        console.log("Video to frame...");
        flipable.toFrame();
    }

    function doImageFullscreen() {
        console.log("Image to fullscreen...");
        crossImage.toFullscreen();
    }

    function doImageFrame() {
        console.log("Image to frame...");
        crossImage.toFrame();
    }

    function flip() {
        console.log("Flipping...");
        flipable.flipped = !flipable.flipped;
    }
}
