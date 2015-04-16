import QtQuick 2.0
import QtMultimedia 5.0

Rectangle {
	anchors.fill: parent

	Video {
		id: myVideo
		anchors.fill: parent
		muted: false
		//autoPlay:true
		//source: "file:///home/pi/big_buck_bunny_1080p_h264.mov"

		onPlaybackStateChanged: {
			if (playbackState === MediaPlayer.PlayingState)
				myTimer.start();
		}
	}

	Timer {
		property int loops: 0

		id: myTimer
		interval: 10000
		repeat: true
		running: false
		triggeredOnStart: false
		onTriggered: {
			loops++;
			var index = loops%5;
			var duration = myVideo.duration;
			var position = index/5*duration;

			myVideo.seek(position);
			logger.info("Seeked " + loops + " times. Seeked to " + position + ".");
		}
	}

	Item {
		id: mediaOutput
		objectName: "mediaOutput"

		function showUrlMedia(uri) {
			myVideo.source = uri;
			myVideo.play();
		}
	}
}
