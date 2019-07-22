PiOmxTextures (POT)
===================
This is a proof of concept of how to build a QML Qt component that renders h264 1080p
video and audio using Raspberry Pi hardware decoder. Also contains code to hardware-decode
images to QML components.

In the piomxtextures_qt_driver there is a QtMultimedia backend sample using POT as a shared
library.

In piomxtextures_poxplayer there is a sample player used to test the backend.

For more information on the project refer to http://goo.gl/KphzdD.<br/>
For some build instructions: http://goo.gl/9TZPxn.<br/>
For some info on how to use: http://goo.gl/TuiSyS.

For a demo: https://www.youtube.com/watch?v=SeJxQN-W2uA.

POT releases include builds that provide Qt Multimedia Backend, POT library, sample apps and Qt builds of most available Qt modules and, sometimes, complete firmware images (see http://goo.gl/KphzdD).
I'll keep the project up as long as I'll see interest for it. Enjoy! ;-)

<span class="badge-paypal"><a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=ZGPF5H6F8N7FS" title="Donate to this project using Paypal"><img src="https://img.shields.io/badge/paypal-donate-yellow.svg" alt="PayPal donate button" /></a></span>

Yocto image
===========
There is a recipe to include POT plugin into your Yocto images: https://github.com/carlonluca/meta-pot.git.
