/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.31.2013
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
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0

POC_AnimatedOverlay {
    signal urlSelected(string url)

    height: parent.height*1/5

    // The content of the view.
    Row {
        anchors.verticalCenter:   parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        height:  parent.height/2
        width:   parent.width - spacing*2
        spacing: 20

        // The open button.
        Button {
            id: buttonOpen
            anchors.verticalCenter: parent.verticalCenter
            style: ButtonStyle {
                label: Text {
                    text:   qsTr("Open URL")
                    width:  buttonOpen.width
                    height: buttonOpen.height
                    horizontalAlignment: Text.AlignHCenter
                    color:  "white"
                }
            }

            // Emit signal.
            onClicked: parent.parent.urlSelected(urlInput.text)
        }

        // The text box.
        TextInput {
            id:    urlInput
            width:  parent.parent.width*4/5 - 2*parent.spacing
            anchors.verticalCenter: parent.verticalCenter
            color:  "white"
            font.pixelSize: 20
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape)
                parent.hideAnimated();
            else if (event.key === Qt.Key_Return)
                parent.urlSelected(urlInput.text);
            else if (event.key === Qt.Key_Enter)
                parent.urlSelected(urlInput.text);
            event.accepted = true;
        }
    }

    // Focus to the input box immediately.
    onFocusChanged: {
        if (activeFocus)
            urlInput.focus = true;
    }
}
