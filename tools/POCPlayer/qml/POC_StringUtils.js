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

/**
  * Converts a number of seconds to a string with the format
  * HH:MM:SS.
  * @param s The number of seconds.
  */
function secondsToHHMMSS(s) {
    var sec_num = Math.round(s);
    var hours   = Math.floor(sec_num/3600);
    var minutes = Math.floor((sec_num - (hours*3600))/60);
    var seconds = sec_num - (hours*3600) - (minutes*60);

    if (hours   < 10)
        hours = "0" + hours;
    if (minutes < 10)
        minutes = "0" + minutes;
    if (seconds < 10)
        seconds = "0" + seconds;

    var time = hours + ':' + minutes + ':' + seconds;
    return time;
}
