/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    05.02.2015
 *
 * Copyright (c) 2015 Luca Carlon. All rights reserved.
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

Rectangle {
	anchors.fill: parent
	color: "red"

	Video {
		id: video1
		x: 0; y: 0; width: parent.width; height: parent.height
		opacity: 1
		fillMode: VideoOutput.Stretch
		muted: opacity == 0
		onStopped: play()

		Behavior on opacity {NumberAnimation {duration: 1000}}
		Behavior on x {NumberAnimation {duration: 1000}}
		Behavior on y {NumberAnimation {duration: 1000}}
	}

	Video {
		id: video2
		x: 0; y: 0; width: parent.width; height: parent.height
		opacity: 0
		fillMode: VideoOutput.Stretch
		muted: opacity == 0
		onStopped: play()

		Behavior on opacity {NumberAnimation {duration: 1000}}
		Behavior on x {NumberAnimation {duration: 1000}}
		Behavior on y {NumberAnimation {duration: 1000}}
	}

	SequentialAnimation {
		id: switchAnim
		ScriptAction {
			script: {
				video1.opacity = 1
				video2.opacity = 2
				video1.x = -video1.width/2
				video2.x = video2.width/2
				video1.y = -video1.height/2
				video2.y = video2.height/2
			}
		}
	}

	Timer {
		interval: 5000
		repeat: true
		running: true
		onTriggered: {
			video1.opacity = video1.opacity == 0 ? 1 : 0
			video2.opacity = video2.opacity == 0 ? 1 : 0
			//switchAnim.running = true
		}
	}

	function showLocalMedia(list) {
		logger.info("Setting " + list[0])
		logger.info("Setting " + list[1])

		video1.source = "file://" + list[0]
		video2.source = "file://" + list[1]

		video1.play()
		video2.play()
	}
}
