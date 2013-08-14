/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.29.2013
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
import "POC_Constants.js" as POC_Constants

Rectangle {
    color: "transparent"

    // Used to handle click events.
    MouseArea {
        anchors.fill: parent
        onReleased: {
            handleUserChoice();
            mouse.accepted = true;
        }
    }

    // The actual element.
    Column {
        width:  parent.width
        height: parent.height

        Image {
            id:       filePreview
            width:    gridBrowser.cellWidth
            height:   gridBrowser.cellHeight - fileText.height - 10
            fillMode: Image.PreserveAspectFit

            // Do not use the source property directly. Use this handler instead so that
            // it is possible to decide the proper icon to use.
            Component.onCompleted: {
                if (fileIsDir) {
                    source = POC_Constants.PATH_ICON_DIR;
                    return;
                }

                if (utils.isSupportedAudio(filePath))
                    source = POC_Constants.PATH_ICON_AUDIO;
                else if (utils.isSupportedVideo(filePath))
                    source = POC_Constants.PATH_ICON_VIDEO;
                else if (utils.isSupportedImage(filePath))
                    source = POC_Constants.PATH_ICON_IMAGE;
                else
                    source = POC_Constants.PATH_ICON_UNKNOWN;
            }
        }

        Text {
            id:    fileText
            text:  fileName
            width: gridBrowser.cellWidth
            elide: Text.ElideMiddle
            color: "white"
            clip:  true
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Enter)
            handleUserChoice();
        else if (event.key === Qt.Key_Return)
            handleUserChoice();
    }

    // Handle user choice.
    function handleUserChoice() {
        if (fileIsDir)
            fileBrowser.currentDir = filePath;
        else
            fileBrowser.fileSelected(filePath);
    }
}
