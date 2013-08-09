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
      Shows a video on the media output.
      */
    function showVideo(videoUri) {
        state = "VIDEO";
        mediaPlayer.source = videoUri;
        mediaPlayer.play();
        videoOutput.focus  = true;
    }

    /**
      Shows an image on the media output.
      */
    function showImage(imageUri) {
        state = "IMAGE";
        mediaPlayer.stop();
        imageOutput.source = imageUri;
        imageOutput.focus  = true;
    }

    /**
      Method used to "go on" to next media or inside the media.
      */
    function goOnMedia() {
        if (state === "VIDEO")
            videoOutput.goOnMedia();
        else if (state === "IMAGE")
            imageOutput.goOnMedia();
    }

    /**
      Method to "go back" to prev media or inside the media.
      */
    function goBackMedia() {
        if (state === "VIDEO")
            videoOutput.goBackMedia();
        else if (state === "IMAGE")
            imageOutput.goBackMedia();
    }

    onFocusChanged: {
        console.log("MediaOutput focus: " + activeFocus);
        if (activeFocus && state === "VIDEO")
            videoOutput.focus = true;
        else if (activeFocus && state === "IMAGE")
            imageOutput.focus = true;
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
}
