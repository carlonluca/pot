#!/bin/sh
#
# Project: OpenMAXIL QtMultimedia Plugin
# Company: -
# Author:  Luca Carlon (carlon.luca (AT) gmail.com)
# Date:    04.21.2013
#

# This script can be used to prepare everything you need to compile the OpenMAXIL QtMultimedia backend
# into Qt sources. Run this script and PiOmxTextures and ffmpeg libs will be built and placed into
# PiOmxTextures/openmaxil_backend/3rdparty.
# First, remember to run the compile_ffmpeg.sh script to download and build ffmpeg.

# Check env variable.
: ${QMAKE_PATH:?"Need to set QMAKE_PATH to the qmake path."}

# Check the user provided the number of threads to use when building.
if [ $# -ne 1 ]; then
   echo "Illegal arguments. Please provide just one parameter with the number of parallel threads to use when building."
   exit
fi

echo "This script will automatically download and build the dependencies..."

if [ ! -d ../3rdparty/ffmpeg/ffmpeg_pi1/lib ] && [ ! -d ../3rdparty/ffmpeg/ffmpeg_pi2/lib ]; then
   echo "It seems you did not compile ffmpeg. Please run compile_ffmpeg.sh first."
   exit
elif [ ! -d ../3rdparty/ffmpeg/ffmpeg_pi1/include ] && [ ! -d ../3rdparty/ffmpeg/ffmpeg_pi2/include ]; then
   echo "It seems you did not compile ffmpeg. Please run compile_ffmpeg.sh first."
   exit
fi

while true; do
    read -p "The openmaxil_backend/3rdparty dir will be cleared. Are you sure you want to go on? " yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done

echo "Preparing 3rdparty dir..."
cd ..
if [ ! -d openmaxil_backend/3rdparty ]; then
   mkdir -p openmaxil_backend/3rdparty
fi
cd openmaxil_backend/3rdparty
rm -rf PiOmxTextures
rm -rf ffmpeg

echo "Ok, about to compile PiOmxTextures..."
if [ ! -d build-PiOmxTextures ]; then
   mkdir build-PiOmxTextures
fi

cd build-PiOmxTextures
$QMAKE_PATH "DEFINES+=CONFIG_LIB" "DEFINES+=CONFIG_INCLUDE_FFMPEG" ../../..
make -j$1
make install

echo "PiOmxTextures built. Copying libs and headers..."
cd ../
mkdir -p PiOmxTextures/lib
mkdir -p PiOmxTextures/include
mkdir ffmpeg

cp -a build-PiOmxTextures/libPiOmxTextures* PiOmxTextures/lib
cp -a build-PiOmxTextures/piomxtextures/* PiOmxTextures/include
cp -a ../../3rdparty/ffmpeg/include ffmpeg
cp -a ../../3rdparty/ffmpeg/lib ffmpeg

# Clone LightLogger
rm -rf LightLogger
git clone https://github.com/carlonluca/LightLogger.git

echo "Cleaning up..."
rm -rf build-PiOmxTextures
