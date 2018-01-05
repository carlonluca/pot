#!/bin/bash

function show_help {
	echo "Usage: prepare_3rdparty.sh <qt_src_root> pi1|pi2|pi3"
	exit 0
}

if [ $# -lt 2 ]; then
	echo "Illegal arguments."
	show_help
fi

# .qmake.conf is needed to build the Qt plugin.
THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
echo "SRC_DIR=$1" > "$THIS_DIR/../piomxtextures_qt_driver/mediaplayer/.qmake.conf"

mkdir -p "$THIS_DIR/../3rdparty"
cd "$THIS_DIR/../3rdparty/"

"$THIS_DIR/compile_ffmpeg.sh" $2

cd "$THIS_DIR"

echo "All done. Bye bye ;-)"
