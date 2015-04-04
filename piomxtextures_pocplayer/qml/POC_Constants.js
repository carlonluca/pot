/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.15.2013
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

.pragma library

// Paths for icons.
var PATH_ICON_DIR     = "qrc:///img/airglass-inode-directory.png";
var PATH_ICON_UNKNOWN = "qrc:///img/unknown.png";
var PATH_ICON_IMAGE   = "qrc:///img/image-x-generic.png"
var PATH_ICON_VIDEO   = "qrc:///img/video-x-generic.png"
var PATH_ICON_AUDIO   = "qrc:///img/audio-x-generic.png"

/**
  * Represents the possible orientations.
  */
var Orientation = {
    HORIZONTAL:        {value: 0, rotation: 0},
    VERTICAL:          {value: 1, rotation: 90.0},
    HORIZONTAL_UPDOWN: {value: 2, rotation: 180.0},
    VERTICAL_UPDOWN:   {value: 3, rotation: 270.0}
};
if (Object.freeze)
    Object.freeze(Orientation);
