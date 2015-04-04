import QtQuick 2.0

Item {
    id: mainItem

    property var source1: null
    property var source2: null
    property var current: image1

    Image {
        id: image1
        source: source1

        x: 0
        y: 0
        width: mainItem.width
        height: mainItem.height

        opacity: 1.0

        Behavior on opacity {
            NumberAnimation {
                duration: 1000
            }
        }
    }

    Image {
        id: image2
        source: source2

        x: 0
        y: 0
        width: mainItem.width
        height: mainItem.height

        opacity: 2.0

        Behavior on opacity {
            NumberAnimation {
                duration: 1000
            }
        }
    }

    function next() {
        if (current === image1) {
            current = image2;
            image1.opacity = 0.0;
            image2.opacity = 1.0;
            return;
        }

        current = image1;
        image1.opacity = 1.0;
        image2.opacity = 0.0;
    }
}
