import QtQuick 2.4
import QtMultimedia 5.0

Rectangle {
	//property string prefix: "file:///Users/luca/test_samples/1080p/"
	//property variant playlist: [
	//	prefix + "big_buck_bunny_1080p_h264_10s.mp4",
	//	prefix + "sintel_trailer_1080p.mp4"
	//];
	property variant playlist
	property int index: 0
	property int dist: 20
	property int duration: 1000

	id: mainRectangle
	color: "red"

	width: parent === null ? 100 : parent.width
	height: parent === null ? 100 : parent.height

	Component.onCompleted: {
                var arguments = Qt.application.arguments;
                if (arguments.length < 4) {
                        console.log("Too few arguments.");
                        Qt.quit();
                }

                var mediaList = [];
                for (var i = 2; i < arguments.length; i++)
                        mediaList.push(arguments[i]);

                playlist = mediaList;
		switchToNext();
        }

	Video {
		id: video1

		x: 0
		y: 0
		width: mainRectangle.width
		height: mainRectangle.height

		onStopped: {
			console.log("State changed for video1 to stopped...");
			switchToNext()
		}

		Component.onCompleted: {
			switchToNext();
		}
	}

	Video {
		id: video2

		x: 0
		y: 0
		width: mainRectangle.width
		height: mainRectangle.height

		onStopped: {
			console.log("State changed for video2 to stopped...");
			switchToNext()
		}
	}

	NumberAnimation {
		id: showAnim
		property: "x"
		from: parent.width + dist
		to: 0
		duration: duration
		easing.type: Easing.InOutQuad
	}

	NumberAnimation {
		id: hideAnim
		property: "x"
		from: 0
		to: -parent.width - dist
		duration: duration
		easing.type: Easing.InOutQuad
	}

	Text {
		id: ticker
		text: "Some loooooooooooooooooong ticker to show!"

		NumberAnimation {
			id: tickerAnim
			target: ticker
			property: "x"
			duration: 5000
			from: mainRectangle.width
			to: -ticker.contentWidth
			loops: Animation.Infinite
		}

		Component.onCompleted: tickerAnim.start()
	}

	function switchToNext() {
		console.log("Switching to " + playlist[index] + "...");
		var currentVideoOutput = index%2 === 0 ? video1 : video2;
		currentVideoOutput.source = playlist[index++%playlist.length];
		currentVideoOutput.play();

		showAnim.target = currentVideoOutput;
		hideAnim.target = currentVideoOutput === video1 ? video2 : video1

		showAnim.start();
		hideAnim.start();
	}

	onWidthChanged: {
		tickerAnim.from = width
		tickerAnim.stop()
		tickerAnim.start()
	}
}
