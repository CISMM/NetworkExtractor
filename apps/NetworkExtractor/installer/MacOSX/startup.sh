#!/bin/sh
#
# Author: Cory Quammen
# Note: This file will be renamed to NetworkExtractor in NetworkExtractor.app/Contents/MacOS/
#

echo "Running NetworkExtractor executable."

FA_BUNDLE="`echo "$0" | sed -e 's/\/Contents\/MacOS\/NetworkExtractor\ [0-9]*.[0-9]*.[0-9]*//'`"
FA_EXECUTABLE="$FA_BUNDLE/Contents/Resources/bin/NetworkExtractor"

export "DYLD_LIBRARY_PATH=$FA_BUNDLE/Contents/Resources/lib/"

export "DYLD_FRAMEWORK_PATH=$FA_BUNDLE/Contents/Frameworks/"

exec "$FA_EXECUTABLE"
