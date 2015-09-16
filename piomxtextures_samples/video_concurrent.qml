/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    08.24.2015
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
import QtQml 2.2
import QtMultimedia 5.0

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
		id: video1
		width: 500
		height: 450
		x: 100
		y: 400
		source: uri1
		autoPlay: true
		onStopped: {video1.seek(0); video1.play();}
	}

	Video {
		id: video2
		x: 900
		y: 400
		width: 500
		height: 450
		source: uri2
		autoPlay: true
		onStopped: {video2.seek(0); video2.play();}
	}

	Timer {
		interval: 1000
		running: true; repeat: true
		onTriggered: {
			console.log("Position video1: " + video1.position + ".");
			console.log("Position video2: " + video2.position + ".");
		}
	}
}
