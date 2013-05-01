#
# Project: PiOmxTextures
# Author:  Luca Carlon
# Date:    01.04.2012
#
#!/bin/bash


# Configures and builds ffmpeg for the PiOmxTextures project. The result is placed in
# <ffmpeg_sources>/ffmpeg_compiled. Both static and shared libraries are compiled.
# Usage:
#    ./compile_ffmpeg.sh <n>, where n is the number of compilation threads to use.

# Check the user provided the number of threads to use when building.
if [ $# -ne 1 ]; then
   echo "Illegal arguments. Please provide just one parameter with the number of parallel threads to use when building."
   exit
fi

echo "Downloading ffmpeg sources from git..."
cd ..
if [ ! -d "3rdparty/ffmpeg" ]; then
   mkdir -p 3rdparty/ffmpeg
fi

cd 3rdparty/ffmpeg
git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg_src
cd ffmpeg_src
git checkout master
git checkout e820e3a2591a0d544925ff1522d2a688a647f1b0

echo "Configuring..."
FLOAT=hard
export PATH=$PATH:/opt/rpi/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/
echo "Prefix to $PWD..."
./configure \
   --extra-cflags="-mfpu=vfp -mfloat-abi=$FLOAT -mno-apcs-stack-check -mstructure-size-boundary=32 -mno-sched-prolog" \
   --enable-cross-compile \
   --enable-shared \
   --enable-static \
   --arch=arm \
   --cpu=arm1176jzf-s \
   --target-os=linux \
   --enable-muxers \
   --enable-encoders \
   --disable-devices \
   --disable-ffprobe \
   --disable-ffplay \
   --disable-ffserver \
   --disable-ffmpeg \
   --enable-doc \
   --enable-postproc \
   --enable-gpl \
   --enable-protocols \
   --enable-pthreads \
   --disable-runtime-cpudetect \
   --enable-pic \
   --disable-armv5te \
   --enable-neon \
   --enable-armv6t2 \
   --enable-armv6 \
   --enable-hardcoded-tables \
   --disable-runtime-cpudetect \
   --disable-debug \
   --cross-prefix=arm-linux-gnueabihf- \
   --prefix=$PWD/ffmpeg_compiled

echo "Compiling..."
mkdir $PWD/ffmpeg_compiled
make -j$1
make install

echo "Cleaning up..."
mv ffmpeg_compiled/include ../
mv ffmpeg_compiled/lib ../

echo "Done! Bye bye! ;-)"
