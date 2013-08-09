import QtQuick 2.0

Image {
    width:  parent.width
    height: parent.height

    /**
      Should "go on" inside the media or to the next media.
      */
    function goOnMedia() {
        source = "file://" + utils.getNextImage(utils.getPathFromUri(source));
    }

    /**
      Should "go back" inside the media or to the prev media.
      */
    function goBackMedia() {
        source = "file://" + utils.getPrevImage(utils.getPathFromUri(source));
    }

    Keys.onPressed: {
        console.log("Pressed!");
        event.accepted = true;
    }
}
