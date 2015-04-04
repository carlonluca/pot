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
import Qt.labs.folderlistmodel 1.0

POC_AnimatedOverlay {
    property var currentDir: utils.getHomeDir()
    signal fileSelected(string fileAbsPath)

    id: fileBrowser

    // Model listing files.
    FolderListModel {
        id:               folderListModel
        folder:           "file://" + currentDir
        showDirsFirst:    true
        showDotAndDotDot: true
        nameFilters:      ["*.*"]
    }

    // Component that shows files.
    Component {
        id: fileDelegate

        POC_FileBrowserItem {
            width:  gridBrowser.cellWidth
            height: gridBrowser.cellHeight
        }
    }

    // The actual element.
    GridView {
        id:           gridBrowser
        anchors.fill: parent
        clip:         true
        model:        folderListModel
        delegate:     fileDelegate
        opacity: 1.0

        cellWidth: 128; cellHeight: height/4;

        highlight:                   highlight
        highlightFollowsCurrentItem: true
        keyNavigationWraps:          true

        // Exit on escape.
        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                focus          = false;
                hideAnimated();
                event.accepted = true;
            }

            event.accepted = false;
        }

        Component {
            id: highlight
            Rectangle {
                width: gridBrowser.cellWidth; height: gridBrowser.cellHeight
                color: "lightsteelblue"; radius: 5
                x: gridBrowser.currentItem.x
                y: gridBrowser.currentItem.y
                Behavior on x { SpringAnimation { spring: 3; damping: 0.2 } }
                Behavior on y { SpringAnimation { spring: 3; damping: 0.2 } }
            }
        }
    }

    // Focus directly to the grid.
    onFocusChanged: {
        if (activeFocus)
            gridBrowser.focus = true;
    }
}
