import QtQuick 2.0
import QtMultimedia 5.4
import QtQuick.Layouts 1.1

Rectangle {
	id: myRect
	anchors.fill: parent

	Video {
		id: myVideo
		anchors.fill: parent

		onPlaybackStateChanged: {
			if (playbackState == MediaPlayer.StoppedState)
				myVideo.play();
		}
	}

	ColumnLayout {
		property var animations: ["yellow", "blue", "violet", "orange"];

		anchors.fill: parent
		spacing: 10

		Rectangle {
			property int index: 0

			id: myRect1

			Layout.preferredWidth: parent.width/2
			Layout.preferredHeight: parent.height/10

			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

			color: "red"

			Behavior on color {ColorAnimation{duration: 3000}}
		}

		Rectangle {
			property int index: 1

			id: myRect2

			Layout.preferredWidth: parent.width/2
			Layout.preferredHeight: parent.height/10

			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

			color: "red"

			Behavior on color {ColorAnimation{duration: 3000}}
		}

		Rectangle {
			property int index: 2

			id: myRect3

			Layout.preferredWidth: parent.width/2
			Layout.preferredHeight: parent.height/10

			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

			color: "red"

			Behavior on color {ColorAnimation{duration: 3000}}
		}

		Timer {
			interval: 10000
			running: true
			repeat: true

			onTriggered: {
				switchColor(myRect1);
				switchColor(myRect2);
				switchColor(myRect3);
			}

			function switchColor(element) {
				element.index++;
				element.color = parent.animations[(element.index)%parent.animations.length];
			}
		}
	}

	Item {
		objectName: "mediaOutput"

		function showUrlMedia(uri) {
			myVideo.source = uri;
			myVideo.play();
		}
	}
}
