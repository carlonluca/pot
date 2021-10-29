import QtQuick 2.5
import QtQuick.Window 2.2
import PiOmxTexturesVideoLayer 0.1

POT_VideoLayer {
	property var playlist: [
		"/home/pi/big_buck_bunny_1080p_h264_10s.mov",
		"/home/pi/las_vegas.mp4",
		"/home/pi/gangsters.mp4"
	]
	property var index: 0

	id: videoLayerItem
	videoLayer: 1
	source: "file://" + playlist[0]

	Component.onCompleted: videoLayerItem.play()

	onDurationReceived:
		console.log("Duration async: " + duration + ".")
	onPositionReceived:
		console.log("Position async: " + position + ".")

	onStopped: {
		console.log("APP: Stopped")
		videoLayerItem.source = "file://" + playlist[++index%2]
		videoLayerItem.play()
	}

	Timer {
		running: true
		interval: 3000
		repeat: true
		onTriggered: {
			// The sync calls are convenient but not optimal as those may
			// block the renderer.
			//console.log("Duration: " + videoLayerItem.duration)
			//console.log("Position: " + videoLayerItem.position)
			videoLayerItem.requestDuration()
			videoLayerItem.requestPosition()
		}
	}

	Rectangle {
		color: "red"
		width: 200
		height: 200
		x: -width
		y: 200

		Text {
			anchors.centerIn: parent
			text: "Hello Video Layer!"
		}

		// Horizontal slow uniform movements are very difficult for the GPU
		// as lack of preciseness during the rendering is clearly visible.

		NumberAnimation on opacity {
			loops: Animation.Infinite
			from: 0.0
			to: 1.0
			running: true
			duration: 5000
		}

		NumberAnimation on x {
			loops: Animation.Infinite
			from: -200.0
			to: Screen.width
			running: true
			duration: 5000
		}
	}
}
