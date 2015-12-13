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
import QtQml 2.2
import QtMultimedia 5.0

Rectangle {
	property var videoUris: []
	property var audioUris: []
	property int videoIndex: 0
	property int audioIndex: 0

	color: "red"

	Component.onCompleted: {
		var arguments = Qt.application.arguments;
		if (arguments.length < 6) {
			console.log("Too few arguments.");
			Qt.quit();
		}

		videoUris.push(arguments[2]);
		videoUris.push(arguments[3]);
		audioUris.push(arguments[4]);
		audioUris.push(arguments[5]);

		nextVideo(video1);
		nextVideo(video2);
		nextAudio(audio);
	}

	Video {
		id: video1
		width: 500
		height: 450
		x: parent.width/3 - width/2
		y: parent.height/2 - height/2
		autoPlay: false
		onStopped: nextVideo(video1)
	}

	Video {
		id: video2
		width: 500
		height: 450
		x: parent.width/3*2 - width/2
		y: parent.height/2 - height/2
		autoPlay: false
		onStopped: nextVideo(video2)
	}

	Audio {
		id: audio
		source: uri3
		autoPlay: false
		onStopped: nextAudio(audio)
	}

	Timer {
		interval: 500
		running: true; repeat: true
		onTriggered: {
			console.log("Position video1: " + video1.position + ".");
			console.log("Position video2: " + video2.position + ".");
			console.log("Position audio: " + audio.position + ".");
		}
	}

	Timer {
		interval: 5000
		running: true; repeat: true
		onTriggered: {
			console.log("Switching audio...");
			audio.stop();
		}
	}

	function nextVideo(element) {
		element.source = videoUris[videoIndex++%2];
		element.play();
	}

	function nextAudio(element) {
		element.source = audioUris[audioIndex++%2];
		element.play();
	}
}
