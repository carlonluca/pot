#!/bin/bash

function show_help {
	echo "Usage: prepare_3rdparty.sh pi1|pi2|pi3"
	exit 0
}

if [ $# -lt 1 ]; then
	echo "Illegal arguments."
	show_help
fi

# .qmake.conf is needed to build the Qt plugin.
THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

mkdir -p "$THIS_DIR/../3rdparty"
cd "$THIS_DIR/../3rdparty/"

"$THIS_DIR/compile_ffmpeg.sh" $1

cd "$THIS_DIR"

echo "All done. Bye bye ;-)"
