import QtQuick 2.0

FocusScope {
    width:  parent.width
    height: parent.height
    state:  "VIDEO"

    // The video output component.
    POC_VideoOutput {
        id:     videoOutput
        source: mediaPlayer
    }

    // The image output.
    POC_ImageOutput {
        id: imageOutput
    }

    /**
      * Determines the type of media and plays it.
      */
    function showLocalMedia(mediaPath) {
        var mediaUri = "file://" + mediaPath;
        if (utils.isSupportedAudio(mediaPath));
            // TODO: Implement!
        else if (utils.isSupportedImage(mediaPath))
            showImage(mediaUri);
        else if (utils.isSupportedVideo(mediaPath))
            showVideo(mediaUri);
        else
            // TODO: Implement dialog here.
            console.log("Can't handle this media, sorry.");
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
        if (state === "VIDEO")
            videoOutput.goOnMedia();
        else if (state === "IMAGE")
            imageOutput.goOnMedia();
    }

    /**
      * Method to "go back" to prev media or inside the media.
      */
    function goBackMedia() {
        if (state === "VIDEO")
            videoOutput.goBackMedia();
        else if (state === "IMAGE")
            imageOutput.goBackMedia();
    }

    onFocusChanged: {
        if (activeFocus && state === "VIDEO") {
            console.log("Giving focus to the video output.");
            videoOutput.focus = true;
        }
        else if (activeFocus && state === "IMAGE") {
            console.log("Giving focus to the image output.");
            imageOutput.focus = true;
        }
        else
            console.log("What kinda status is this?!");
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
