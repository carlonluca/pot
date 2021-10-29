import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.2
import PiOmxTexturesVideoLayer 0.1

Rectangle {
    id: main
    color: "orange"
    anchors.fill: parent

    POT_VideoLayer {
        id: player
        anchors.fill: parent
        source: "file:///home/pi/dolomiti_timelapse_1080p.mkv"
        Component.onCompleted: play()
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        height: parent.height*0.10
        spacing: 20

        Component {
            id: buttonComponent

            Button {
                id: bt
                contentItem: Text {
                    text: parent.text
                    font.pointSize: 30
                    opacity: enabled ? 1.0 : 0.3
                    color: parent.down ? "#17a81a" : "#21be2b"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: main.buttonClicked(bt)
            }
        }
        
        Loader {
            property string text: "Pause"
            id: btPause
            sourceComponent: buttonComponent
            active: true
            width: parent.height
            height: parent.height
            onItemChanged: item.text = text
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
    }

    function buttonClicked(bid) {
        switch (bid) {
            case btPlay.item: { player.play(); break }
            case btPause.item: { player.pause(); break }
            case btStop.item: { player.stop(); break }
        }
    }
}