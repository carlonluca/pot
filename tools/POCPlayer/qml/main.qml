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
import "POC_StringUtils.js" as POC_StringUtils

Rectangle {
    id:     mainView
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

        // Only used for debugging purposes.
        onStopped: {
            console.log("Stopped signal received!");
        }
    }

    // Generic media output for images, videos and audio.
    POC_MediaOutput {
        objectName: "mediaOutput"
        id: mediaOutput

        onControlBarDismissed: parent.focus = true
    }

    // The legend.
    POC_Legend {
        id: legend

        // The element unfocuses.
        onFocusRelinquished: parent.focus = true
    }

    // The metadata.
    POC_MetaData {
        id:      metaData
        source:  mediaPlayer

        // The element unfocuses.
        onFocusRelinquished: parent.focus = true
    }

    // File browser.
    POC_FileBrowser {
        id:         fileBrowser
        currentDir: utils.getHomeDir()

        // When the file is selected, set it as the source of the media
        // player.
        onFileSelected: mediaOutput.showLocalMedia(fileAbsPath)

        // The element unfocuses.
        onFocusRelinquished: parent.focus = true
    }

    // The URL interface.
    POC_UrlInterface {
        id: urlInterface

        onFocusRelinquished: parent.focus = true

        onUrlSelected: mediaPlayer.source = url
    }

    // These are shortcuts for common functionalities.
    Keys.onPressed: {
        console.log("Main interface is processing the key...");
        if (event.key === Qt.Key_S)
            mediaPlayer.stop();
        else if (event.key === Qt.Key_P)
            mediaPlayer.playPause();
        else if (event.key === Qt.Key_Plus)
            mediaPlayer.volumeUp();
        else if (event.key === Qt.Key_Minus)
            mediaPlayer.volumeDown();
        else if (event.key === Qt.Key_Down)
            mediaOutput.showControlBar();
        else if (event.key === Qt.Key_Right)
            mediaOutput.goOnMedia();
        else if (event.key === Qt.Key_Left)
            mediaOutput.goBackMedia();
        else if (event.key === Qt.Key_L)
            legend.toggleVisibility();
        else if (event.key === Qt.Key_T)
            metaData.showAnimated();
        else if (event.key === Qt.Key_O)
            fileBrowser.showAnimated();
        else if (event.key === Qt.Key_U)
            urlInterface.showAnimated();
        else if (event.key === Qt.Key_Q)
            Qt.quit();
        else
            return;

        event.accepted = true;
    }
}
