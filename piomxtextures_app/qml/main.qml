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
    property int indexVideo: 3
    property int indexFlippable: 0
    x: 0
    y: 0
    color: "red"
    focus: true

    Keys.onPressed: {
        if (event.key === Qt.Key_S)
            mediaProcessor.stop();
        else if (event.key === Qt.Key_P)
            mediaProcessor.play();
        else if (event.key === Qt.Key_A)
            mediaProcessor.pause();
        else if (event.key === Qt.Key_O)
            mediaProcessor.source = "big_buck_bunny_1080p_h264.mov";
        else if (event.key === Qt.Key_K)
            mediaProcessor.seek(1000);
    }

    Timer {
        interval: 2000; running: true; repeat: true
        onTriggered: {
            indexVideo++;
            var theState = indexVideo%4;
            if (theState == 0)
                mainRectangle.state = "LARGE";
            else if (theState == 1)
                mainRectangle.state = "NORMAL";
            else if (theState == 2)
                //myFlip.showBack();
                mainRectangle.state = "LARGE";
            else if (theState == 3)
                //myFlip.showFront();
                mainRectangle.state = "NORMAL";

            crossImage1.next();
            //crossImage2.next();
        }
    }

    Timer {
        interval: 5000; running: true; repeat: true
        onTriggered: {
            console.log("Position: " + mediaProcessor.streamPosition + ".");
        }
    }

    OMXMediaProcessor {
        id: mediaProcessor
        //source: "/home/pi/07 Hail Caesar.mp3"
        //source: "/home/pi/Cars2.mkv"
        source: "/home/pi/big_buck_bunny_1080p_h264.mov"
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
            width: 1280
            height: 720
            x: 0
            y: 130
            source: mediaProcessor
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
        id: imageLogo
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
                to: mainRectangle.width
                duration: 10000
            }
        }
    }

    OMX_CrossImage {
        id: crossImage1

        x: 1293
        y: 130

        width: 627
        height: 372

        source1: "qrc:/resources/1.jpg"
        source2: "qrc:/resources/2.jpg"
    }

    OMX_CrossImage {
        id: crossImage2

        x: 1293
        y: 502

        width: 627
        height: 372

        source1: "qrc:/resources/3.jpg"
        source2: "qrc:/resources/4.jpg"
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target:omxVideoSurface
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
                target:omxVideoSurface;
                x: 0; y: 0; width: mainRectangle.width; height: mainRectangle.height;
            }
            PropertyChanges {
                target: myFlip
                x: 0; y: 0; width: mainRectangle.width; height: mainRectangle.height;
            }
        }
    ]

    transitions: [
        Transition {
            from: "NORMAL"
            to: "LARGE"
            PropertyAnimation {property: "width"; duration: 800; easing.type: Easing.OutBounce}
            PropertyAnimation {property: "height"; duration: 800; easing.type: Easing.OutBounce}
            PropertyAnimation {property: "x"; duration: 800; easing.type: Easing.OutBounce}
            PropertyAnimation {property: "y"; duration: 800; easing.type: Easing.OutBounce}
        },
        Transition {
            from: "LARGE"
            to: "NORMAL"
            PropertyAnimation {property: "width"; duration: 800; easing.type: Easing.OutBounce}
            PropertyAnimation {property: "height"; duration: 800; easing.type: Easing.OutBounce}
            PropertyAnimation {property: "x"; duration: 800; easing.type: Easing.OutBounce}
            PropertyAnimation {property: "y"; duration: 800; easing.type: Easing.OutBounce}
        }
    ]
}
