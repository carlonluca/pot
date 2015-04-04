/*
* Project: PiOmxTextures
* Author:  Luca Carlon
* Date:    07.16.2013
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
import "POC_StringUtils.js" as POC_StringUtils

// The duration text.
Text {
    id:   textPosition
    text: formatText();

    function formatText() {
        var position = POC_StringUtils.secondsToHHMMSS(parent.source.position/1000);
        var duration = POC_StringUtils.secondsToHHMMSS(parent.source.duration/1000);
        return position + "/" + duration;
    }
}
