#!/bin/bash

function show_help {
	echo "Usage: prepare_3rdparty.sh <qt_src_root> pi1|pi2"
	exit 0
}

if [ $# -lt 2 ]; then
	echo "Illegal arguments."
	show_help
fi

# .qmake.conf is needed to build the Qt plugin.
THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
echo "SRC_DIR=$1" > "$THIS_DIR/../piomxtextures_qt_driver/mediaplayer/.qmake.conf"

# Clone LightLogger.
echo "Cloning dependency: LightLogger..."
mkdir -p "$THIS_DIR/../3rdparty"
cd "$THIS_DIR/../3rdparty/"
git clone https://github.com/carlonluca/LightLogger.git --depth 1

"$THIS_DIR/compile_ffmpeg.sh" $2

cd "$THIS_DIR"

echo "All done. Bye bye ;-)"
