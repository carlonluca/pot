import QtQuick 2.12
import QtQuick.Controls 2.12
import PiOmxTexturesVideoLayer 0.1

Rectangle {
    property var requestedOrientation: orientation
    property int currentMode: 0
    property var modes: [
        Qt.IgnoreAspectRatio,
        Qt.KeepAspectRatio,
        Qt.KeepAspectRatioByExpanding
    ]

    id: mainElement
    color: "transparent"
    anchors.fill: parent
    transform: Rotation {
        origin.x: width/2
        origin.y: height/2
        angle: {
            switch (orientation) {
            case "1":
                return 90
            case "2":
                return 180
            case "3":
                return 270
            case "0":
            default:
                return 0
            }
        }
    }

    Rectangle {
        color: "orange"
        anchors.centerIn: parent

        width: {
            switch (orientation) {
            case "1":
            case "3":
                return parent.height
            case "0":
            case "2":
            default:
                return parent.width
            }
        }

        height: {
            switch (orientation) {
            case "1":
            case "3":
                return parent.width
            case "0":
            case "2":
            default:
                return parent.height
            }
        }

        Video {
            id: player
            anchors.fill: parent
            source: "file://" + filePath
            autoPlay: true
            orientation: {
                switch (mainElement.requestedOrientation) {
                case "1":
                    return POT_VideoLayer.ROT_90
                case "2":
                    return POT_VideoLayer.ROT_180
                case "3":
                    return POT_VideoLayer.ROT_270
                default:
                    return POT_VideoLayer.ROT_0
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            height: parent.width*0.10
            spacing: 20

            Component {
                id: buttonComponent

                Button {
                    id: bt
                    contentItem: Text {
                        text: parent.text
                        font.pointSize: 25
                        opacity: enabled ? 1.0 : 0.3
                        color: parent.down ? "#17a81a" : "#21be2b"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    onClicked: mainElement.buttonClicked2(bt)
                }
            }

            Button {
                id: btPause
                contentItem: Text {
                    text: "Pause"
                    font.pointSize: 25
                    opacity: enabled ? 1.0 : 0.3
                    color: parent.down ? "#17a81a" : "#21be2b"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: player.pause()
                width: parent.height
                height: parent.height
            }

            Loader {
                property string text: "Play"
                id: btPlay
                sourceComponent: buttonComponent
                active: true
                width: parent.height
                height: parent.height
                onItemChanged: item.text = text
            }

            Loader {
                property string text: "Stop"
                id: btStop
                sourceComponent: buttonComponent
                active: true
                width: parent.height
                height: parent.height
                onItemChanged: item.text = text
            }

            Loader {
                property string text: "Seek"
                id: btSeek
                sourceComponent: buttonComponent
                active: true
                width: parent.height
                height: parent.height
                onItemChanged: item.text = text
            }

            Button {
                id: btMute
                contentItem: Text {
                    text: "Mute"
                    font.pointSize: 25
                    opacity: enabled ? 1.0 : 0.3
                    color: parent.down ? "#17a81a" : "#21be2b"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: player.muted = !player.muted
                width: parent.height
                height: parent.height
            }

            Loader {
                property string text: "Mode"
                id: btFillMode
                sourceComponent: buttonComponent
                active: true
                width: parent.height
                height: parent.height
                onItemChanged: item.text = text
            }
        }
    }

    function buttonClicked2(bid) {
        switch (bid) {
            case btPlay.item: player.play(); break
            case btPause.item: player.pause(); break
            case btStop.item: player.stop(); break
            case btSeek.item: player.seek(5000); break
            case btMute.item: player.muted = !player.muted; break
            case btFillMode.item: player.fillMode = modes[(currentMode++)%3]
        }
    }
}
