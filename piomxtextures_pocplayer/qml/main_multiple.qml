/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    04.11.2015
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
import QtMultimedia 5.4
import QtQuick.Layouts 1.1

Rectangle {
	id: myRect

	anchors.fill: parent
	color: "black"

	GridLayout {
		anchors.fill: parent

		rows: 1
		columns: 2

		Video {
			id: myVideo1
			source: "file:///home/pi/big_buck_bunny_720p_h264.mov"
			autoPlay: true
			muted: true

			width: 700
			height: 700

			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
		}

		Video {
			id: myVideo2
			source: "file:///home/pi/sintel_trailer_720p.mp4"
			autoPlay: true
			muted: false

			width: 700
			height: 700

			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
		}
	}

	Audio {
		id: myAudio
		source: "file:///home/pi/master_of_puppets.mp3"
		autoPlay: true
	}
}
