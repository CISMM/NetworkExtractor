#!/bin/sh
#
# Author: Cory Quammen
# Note: This file will be renamed to FibrinAnalysis in FibrinAnalysis.app/Contents/MacOS/
#

echo "Running FibrinAnalysis executable."

FA_BUNDLE="`echo "$0" | sed -e 's/\/Contents\/MacOS\/FibrinAnalysis//'`"
FA_EXECUTABLE="$FA_BUNDLE/Contents/Resources/bin/FibrinAnalysis"

export "DYLD_LIBRARY_PATH=$FA_BUNDLE/Contents/Resources/lib/"

export "DYLD_FRAMEWORK_PATH=$FA_BUNDLE/Contents/Frameworks/"

exec "$FA_EXECUTABLE"
