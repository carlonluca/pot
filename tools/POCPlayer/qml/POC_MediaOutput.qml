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
    }

    /**
      Shows an image on the media output.
      */
    function showImage(imageUri) {
        state = "IMAGE";
        mediaPlayer.stop();
        imageOutput.source = imageUri;
    }

    onFocusChanged: {
        console.log("Focus: " + activeFocus);
        if (activeFocus)
            videoOutput.focus = true;
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
