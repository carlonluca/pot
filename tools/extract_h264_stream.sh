#
# Project: PiOmxTextures
# Author:  Luca Carlon
# Date:    01.11.2012
#
#!/bin/bash

ffmpeg -i $1 -vcodec copy -bsf h264_mp4toannexb -an -f h264 out.h264
