/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    11.01.2012
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import com.luke.qml 1.0

Rectangle {
    id: mainRectangle
    property int indexVideo: 0
    property int indexFlippable: 0
    width: 1920
    height: 1080
    color: "red"
    focus: true

    Keys.onPressed: {
        if (event.key === Qt.Key_S)
            mediaProcessor.stop();
        else if (event.key === Qt.Key_P)
            mediaProcessor.play();
        else if (event.key === Qt.Key_A)
            mediaProcessor.pause();
    }

    Timer {
        interval: 5000; running: true; repeat: true
        onTriggered: {
            indexVideo++;
            var theState = indexVideo%4;
            if (theState == 0)
                mainRectangle.state = "LARGE";
            else if (theState == 1)
                mainRectangle.state = "NORMAL";
            else if (theState == 2)
                myFlip.showBack();
            else if (theState == 3)
                myFlip.showFront();
        }
    }

    OMXMediaProcessor {
        id: mediaProcessor
        source: "/home/pi/usb/Cars2.mkv"
    }

    Flipable {
        id: myFlip
        x:50
        y:50
        width: 900
        height: 600

        function showFront() {
            rot.angle = 0;
        }

        function showBack() {
            rot.angle = 180;
        }

        transform: Rotation {
            id: rot
            origin.x: 400;
            origin.y: 100;
            axis.x: 0; axis.y: 1; axis.z: 0
            angle: 0

            Behavior on angle {PropertyAnimation{}}
        }

        front: OMXVideoSurface {
            id: omxVideoSurface
            width: 900
            height: 600
            x: 50
            y: 50
            source: mediaProcessor

            SequentialAnimation {
                id: theAnimation
                PropertyAnimation {
                    target: omxVideoSurface
                    property: "opacity"
                    to: 0.0
                    duration: 1000
                }
                PropertyAnimation {
                    target: omxVideoSurface
                    property: "opacity"
                    to: 1.0
                    duration: 1000
                }
            }
        }

        back: Item {
            Rectangle {
                width: 800
                height: 430
                color: "green"
            }

            Text {
                x: 30
                y:200
                text: "Turn for the cool side!"
                font.pixelSize: 60
                font.weight: Font.Bold
            }
        }
    }

    Image {
        x: 1500
        y: 100
        width: 300
        height: 300
        source: "qrc:/resources/qt-logo.png"
        opacity: 0.5
    }

    Text {
        id: myMarquee
        text: "Marquee effect here!"
        x: 0
        y: 900

        font.pixelSize: 80
        font.weight: Font.Bold

        SequentialAnimation {
            loops: Animation.Infinite
            running: true
            SmoothedAnimation {
                target: myMarquee
                property: "x"
                from: 0
                to: 1920
                duration: 10000
            }
        }
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: omxVideoSurface
                x: 50; y: 50; width: 900; height: 600;
            }
            PropertyChanges {
                target: myFlip
                x: 50; y: 50; width: 900; height: 600;
            }
        },
        State {
            name: "LARGE"
            PropertyChanges {
                target: omxVideoSurface;
                x: 0; y: 0; width: 1920; height: 1080;
            }
            PropertyChanges {
                target: myFlip
                x: 0; y: 0; width: 1920; height: 1080;
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {properties: "width,height"; duration: 800; easing.type: Easing.OutBounce}
            NumberAnimation {properties: "x,y"; duration: 800}
        }

    ]
}
