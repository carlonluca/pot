import QtQuick 2.0
import QtMultimedia 5.0

Rectangle {
	width: parent.width
	height: parent.height
	color: "red"

	Video {
		id: video
		width: parent.width
		height: parent.height
		source: "file:///home/pi/big_buck_bunny_1080p_h264.mov"
		autoPlay: true
	}

	Audio {
		id: myAudio
		source: "file:///home/pi/win.wav"
		autoPlay: false
	}

	Timer {
		interval: 1000
		running: true; repeat: true
		onTriggered: {myAudio.stop(); myAudio.play()}
	}

	Timer {
		interval: 5000
		running: true; repeat: true
		onTriggered: {video.seek(0)}
	}
}
