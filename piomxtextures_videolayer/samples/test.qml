import QtQuick 2.5
import QtQuick.Window 2.2
import PiOmxTexturesVideoLayer 0.1

POT_VideoLayer {
   property var playlist: [
      "/home/pi/big_buck_bunny_1080p_h264.mov",
      "/home/pi/1.mp4",
      "/home/pi/2.mp4"
   ]
   property var index: 0

   id: videoLayerItem
   videoLayer: 1
   source: "file://" + playlist[0]

   Component.onCompleted:
      videoLayerItem.play()

   onDurationReceived:
      console.log("Duration (async): " + d + ".")
   onPositionReceived:
      console.log("Position (async): " + p + ".")
   onSourceChanged:
      console.log("Source changed to: " + source + ".")

   onStopped: {
      console.log("APP: Stopped")
      videoLayerItem.source = "file://" + playlist[++index%3]
      videoLayerItem.play()
   }

   // This only simulates a stop() command.
   Timer {
      running: true; interval: 13000; repeat: true
      onTriggered:
         videoLayerItem.stop()
   }

   // This simulates props.
   Timer {
      running: true; interval: 3000; repeat: true
      onTriggered: {
         // The sync calls are convenient but not optimal as those may
         // block the renderer.
         measure(function() {
            console.log("Duration (sync): " + videoLayerItem.duration)
            console.log("Position (sync): " + videoLayerItem.position)
         })

         console.log("Requesting pos/length async...")
         measure(function() {
            videoLayerItem.requestDuration()
            videoLayerItem.requestPosition()
         })
      }
   }

   Rectangle {
      color: "red"; width: 200; height: 200
      x: -width; y: 200

      Text {
         anchors.centerIn: parent
         text: "Hello Video Layer!"
      }

      // Horizontal slow uniform movements are very difficult for the GPU
      // as lack of preciseness during the rendering is clearly visible.
      NumberAnimation on opacity {
         loops: Animation.Infinite
         from: 0.0; to: 1.0
         running: true; duration: 5000
      }

      NumberAnimation on x {
         loops: Animation.Infinite
         from: -200.0; to: Screen.width
         running: true; duration: 5000
      }
   }

   function measure(f) {
      var a = new Date().valueOf()
      f()
      var b = new Date().valueOf()
      console.log("Elapsed: " + (b - a) + "ms.")
   }
}
