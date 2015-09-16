/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    08.08.2015
 *
 * Copyright (c) 2012-2015 Luca Carlon. All rights reserved.
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

import QtQuick 2.0
import QtMultimedia 5.0

/**
 * Sample that accepts video URI and audio URI.
 */
Rectangle {
	property string uri1
	property string uri2

	color: "red"

	Component.onCompleted: {
		var arguments = Qt.application.arguments;
		if (arguments.length < 4) {
			console.log("Too few arguments.");
			Qt.quit();
		}

		uri1 = arguments[2];
		uri2 = arguments[3];
	}

	Video {
		id: video
		width: parent.width
		height: parent.height
		source: uri1
		autoPlay: true
	}

	Audio {
		id: myAudio
		source: uri2
		autoPlay: false
	}

	Timer {
		interval: 3000
		running: true; repeat: true
		onTriggered: {myAudio.stop(); myAudio.play()}
	}

	Timer {
		interval: 5000
		running: true; repeat: true
		onTriggered: {video.seek(0)}
	}
}
