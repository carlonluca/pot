import QtQuick 2.0
import QtMultimedia 5.0

Rectangle {
	property int index: 0
	property int count: 0
	property var playlist

	anchors.fill: parent

	Component.onCompleted: {
		var arguments = Qt.application.arguments;
		if (arguments.length < 3) {
			console.log("Too few arguments.");
			Qt.quit();
		}

		var mediaList = [];
		for (var i = 2; i < arguments.length; i++)
			mediaList.push(arguments[i]);

		showMedia(mediaList);
	}

	MediaPlayer {
		id: mediaPlayer

		onStopped:
			 goToNextMedia();
	}

	VideoOutput {
		anchors.fill: parent
		source: mediaPlayer
		fillMode: VideoOutput.Stretch
	}

	Timer {
		running: true;
		interval: 10;
		repeat: true;
		onTriggered: {
			console.log("Position: " + mediaPlayer.position + ".");
		}
	}

	function showMedia(mediaList) {
		for (var i = 0; i < mediaList.length; i++)
			console.log("" + mediaList[i] + " added to the playlist.");
		playlist = mediaList;
		goToNextMedia();
	}

	function goToNextMedia() {
		console.log("Play count: " + count + ".");
		console.log("Opening video file: " + playlist[index] + ".");
		//console.log("Uptime: " + uptime.uptimeString() + ".");

		mediaPlayer.source = playlist[index];
		mediaPlayer.play();

		count++;
		index++;
		index = index >= playlist.length ? 0 : index;
	}
}
