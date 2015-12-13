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

import QtQuick 2.2
import QtQml 2.2
import QtQuick.Window 2.0
import QtMultimedia 5.0

/**
 * This sample creates a Video surface including a MediaPlayer, plays continuously
 * and asks for the current position every 10ms.
 */
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

		uri = arguments[2];
	}

	Video {
		id: myVideo
		anchors.fill: parent
		visible: true

		source: uri
		autoPlay: true

		//onStopped: {
		//	console.log("Stopped.");
		//	myVideo.seek(0);
		//	myVideo.play();
		//}

		onStatusChanged: {
			console.log("Status changed to: " + status + ".");
		}
	}

	Timer {
		property int seekCount: 0

		id: videoInterval
		interval: 500
		repeat: true
		running: true

		onTriggered: {
			console.log("Seek n. " + seekCount++ + "...");
			randomSeek(myVideo);
		}
	}

	function randomSeek(element) {
                var length = element.duration - 1000;
                var randomPosition = Math.random()*length;
                element.seek(randomPosition);
        }
}
