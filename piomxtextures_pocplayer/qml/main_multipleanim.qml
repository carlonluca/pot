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
	property var videoFront: video1
	property var videoBack: video2
	property int durationAnim: 3000

	anchors.fill: parent
	color: "black"

	Video {
		id: video1
		x: 0; y: 0; z: 1; width: parent.width; height: parent.height
		opacity: 1
		fillMode: VideoOutput.Stretch
		onStopped: play()
		volume: z

		Behavior on opacity {NumberAnimation {duration: durationAnim/2}}
		Behavior on x {NumberAnimation {duration: durationAnim/2; easing.type: Easing.InOutCubic}}
		Behavior on y {NumberAnimation {duration: durationAnim/2; easing.type: Easing.InOutCubic}}
		Behavior on scale {NumberAnimation {duration: durationAnim/2; easing.type: Easing.InOutCubic}}
		Behavior on z {NumberAnimation {duration: durationAnim}}
	}

	Video {
		id: video2
		x: 0; y: 0; z: 0; width: parent.width; height: parent.height
		opacity: 0
		fillMode: VideoOutput.Stretch
		onStopped: play()
		volume: z

		Behavior on opacity {NumberAnimation {duration: durationAnim/2}}
		Behavior on x {NumberAnimation {duration: durationAnim/2; easing.type: Easing.InOutCubic}}
		Behavior on y {NumberAnimation {duration: durationAnim/2; easing.type: Easing.InOutCubic}}
		Behavior on scale {NumberAnimation {duration: durationAnim/2; easing.type: Easing.InOutCubic}}
		Behavior on z {NumberAnimation {duration: durationAnim}}
	}

	SequentialAnimation {
		id: switchAnim

		ScriptAction {
			script: {
				logger.debug("Moving 1...");

				videoFront.z = 0;
				videoBack.z = 1;

				videoFront.opacity = 1.0;
				videoBack.opacity = 1.0;

				videoFront.scale = 1.0
				videoBack.scale = 1.0;

				videoFront.x = -videoFront.width/2;
				videoBack.x = videoBack.width/2;
				videoFront.y = -videoFront.height/2;
				videoBack.y = videoBack.height/2;
			}
		}


		PauseAnimation {
			duration: durationAnim/2
		}

		ScriptAction {
			script: {
				logger.debug("Moving 2...");

				videoFront.scale = 0.5;
				videoBack.scale = 1.0;

				videoFront.x = 0;
				videoBack.x = 0;
				videoFront.y = 0;
				videoBack.y = 0;
			}
		}

		PauseAnimation {
			duration: durationAnim/2
		}

		ScriptAction {
			script: {
				logger.debug("Done...");
				switchVideoRefs();
			}
		}
	}

	Timer {
		property int i: 0
		property bool j: true

		interval: 5000
		repeat: true
		running: true
		onTriggered: {
			if (j)
				switchZ();
			else
				switchOpacity();

			if (i%2 === 0)
				j = !j;
			i++
		}
	}

	function switchOpacity() {
		video1.opacity = (videoFront === video1) ? 0 : 1
		video2.opacity = (videoFront === video1) ? 1 : 0
		switchVideoRefs();
	}

	function switchZ() {
		switchAnim.running = true;
	}

	function switchVideoRefs() {
		var tmp = videoFront;
		videoFront = videoBack;
		videoBack = tmp;
	}

	function showLocalMedia(list) {
		logger.info("Setting " + list[0])
		logger.info("Setting " + list[1])

		video1.source = list[0]
		video2.source = list[1]

		video1.play()
		video2.play()
	}
}
