/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    03.19.2016
 *
 * Copyright (c) 2019 Luca Carlon. All rights reserved.
 *
 * This file is part of PiOmxTextures.
 *
 * PiOmxTextures is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PiOmxTextures is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import QtQml 2.0
import QtQuick.Window 2.1
import QtMultimedia 5.0
import PiOmxTexturesQmlUtils 0.1

Rectangle {
	property string uri

	id: root
	objectName: "root"
	visible: true
	color: "black"

	Component.onCompleted: {
		var arguments = Qt.application.arguments;
		if (arguments.length < 3) {
			console.log("Too few arguments.");
			Qt.quit();
		}

		uri = arguments[1];
	}

	VideoOutput {
		id: myVideo
		anchors.fill: parent
		visible: true
		source: mediaPlayer
	}

	MediaPlayer {
		id: mediaPlayer
		source: uri
		autoPlay: true

		onStopped: {
			console.log("Looping...")
			mediaPlayer.play()
		}
	}

	POT_VideoProbe {
		source: mediaPlayer
		onVideoFrameProbed: console.log("Frame probed!")
	}
}
