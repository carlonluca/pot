import QtQuick 2.0
import QtWebEngine 1.2

Rectangle {
	anchors.fill: parent
	color: "red"

	WebEngineView {
		objectName: "webEngineView"
		anchors.fill: parent
	}
}
