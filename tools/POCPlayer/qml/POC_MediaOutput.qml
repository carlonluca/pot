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

    property var currentOutput: videoOutput

    width:  parent.width
    height: parent.height
    state:  "VIDEO"

    // The video output component.
    POC_VideoOutput {
        id:     videoOutput
        source: mediaPlayer

        onControlBarDismissed: parent.controlBarDismissed()
    }

    // The image output.
    POC_ImageOutput {
        id: imageOutput
        opacity: 0.0
        enabled: false

        onControlBarDismissed: parent.controlBarDismissed()
    }

    /**
      * Determines the type of media and plays it.
      */
    function showLocalMedia(mediaPath) {
        var mediaUri = "file://" + mediaPath;
        showUrlMedia(mediaUri);
    }

    function showUrlMedia(mediaUri) {
        if (utils.isSupportedAudio(mediaUri));
            // TODO: Implement!
        else if (utils.isSupportedImage(mediaUri))
            showImage(mediaUri);
        else if (utils.isSupportedVideo(mediaUri))
            showVideo(mediaUri);
        else
            // TODO: Implement dialog here.
            console.log("Can't handle this media, sorry.");
    }

    /**
      * Shows and give focus to the control bar of the currently visible output
      * component.
      */
    function showControlBar() {
        currentOutput.showControlBar();
    }

    /**
      * Shows a video on the media output.
      */
    function showVideo(videoUri) {
        state = "VIDEO";
        mediaPlayer.source = videoUri;
        mediaPlayer.play();
    }

    /**
      * Shows an image on the media output.
      */
    function showImage(imageUri) {
        mediaPlayer.stop();
        state = "IMAGE";
        imageOutput.showImage(imageUri);
    }

    /**
      * Method used to "go on" to next media or inside the media.
      */
    function goOnMedia() {
        currentOutput.goOnMedia();
    }

    /**
      * Method to "go back" to prev media or inside the media.
      */
    function goBackMedia() {
        currentOutput.goBackMedia();
    }

    onStateChanged: {
        // Disable previous output surface.
        currentOutput.enabled = false;

        // Switch.
        switch (state) {
        case "VIDEO":
            currentOutput = videoOutput;
            break;
        case "IMAGE":
            currentOutput = imageOutput;
            break;
        default:
            console.log("Unknown media output status!");
            break;
        }

        // Enable the new one.
        currentOutput.enabled = true;
    }

    states: [
        State {
            name: "VIDEO"
            PropertyChanges {
                target: videoOutput
                opacity: 1.0
            }
            PropertyChanges {
                target: imageOutput
                opacity: 0.0
            }
        },
        State {
            name: "IMAGE"
            PropertyChanges {
                target:  videoOutput
                opacity: 0.0
            }
            PropertyChanges {
                target:  imageOutput
                opacity: 1.0
            }
        }
    ]

    transitions: [
        Transition {
            from: "VIDEO"
            to:   "IMAGE"

            NumberAnimation {
                target:      videoOutput
                property:    "opacity"
                duration:    1000
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                target:      imageOutput
                property:    "opacity"
                duration:    1000
                easing.type: Easing.InOutQuad
            }
        },

        Transition {
            from: "IMAGE"
            to:   "VIDEO"

            NumberAnimation {
                target:      videoOutput
                property:    "opacity"
                duration:    1000
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                target:      imageOutput
                property:    "opacity"
                duration:    1000
                easing.type: Easing.InOutQuad
            }
        }
    ]
}
