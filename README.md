PiOmxTextures
=============
This is a proof of concept of how to build a QML Qt component that renders h264 1080p
video and audio using Raspberry Pi hardware decoder. Also contains code to hardware-decode
images to QML components.
Notes on the project and on how to build it can be found here: http://thebugfreeblog.blogspot.it/2013/02/qml-components-for-video-decoding-and.html.

In the omxplayeril_backend there is a QtMultimedia backend sample using PiOmxTextures as a shared
library. For more information on this refer to: http://thebugfreeblog.blogspot.it/2013/04/hardware-accelerated-qtmultimedia.html.

In tools/POCPlayer there is a sample player used to test the backend.
