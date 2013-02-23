#
# Project: PiOmxTextures
# Author:  Luca Carlon
# Date:    01.11.2012
#
#!/bin/bash

ffmpeg -i $1 -vn -c:a copy somefile.aac
