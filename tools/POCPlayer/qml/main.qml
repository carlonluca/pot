/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.13.2013
*
* Copyright (c) 2012, 2013 Luca Carlon. All rights reserved.
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
    color:  "black"
    focus:  true

    // The unique media player.
    MediaPlayer {
        objectName: "mediaPlayer"
        id: mediaPlayer
        autoLoad: true
        autoPlay: false

        function playPause() {
            if (mediaPlayer.playbackState === MediaPlayer.PlayingState)
                mediaPlayer.pause();
            else if (mediaPlayer.playbackState === MediaPlayer.PausedState)
                mediaPlayer.play();
            else if (mediaPlayer.playbackState === MediaPlayer.StoppedState)
                mediaPlayer.play();
        }

        function volumeUp() {
            volume = (volume + 0.1 > 1) ? 1 : volume + 0.1;
        }

        function volumeDown() {
            volume = (volume - 0.1 < 0) ? 0 : volume - 0.1;
        }
    }

    POC_VideoOutput {
        source: mediaPlayer
    }

    // These are shortcuts for common functionalities.
    Keys.onPressed: {
        if (event.key === Qt.Key_S)
            mediaPlayer.stop();
        else if (event.key === Qt.Key_P)
            mediaPlayer.playPause();
        else if (event.key === Qt.Key_Space)
            mediaPlayer.playPause();
        else if (event.key === Qt.Key_Plus)
            mediaPlayer.volumeUp();
        else if (event.key === Qt.Key_Minus)
            mediaPlayer.volumeDown();
    }
}
