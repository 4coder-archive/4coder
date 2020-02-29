#!/bin/bash

# If any command errors, stop the script
set -e

# Set up directories

ME="$(readlink -f "$0")"
LOCATION="$(dirname "$ME")"
SRC_ROOT="$(dirname "$LOCATION")"
PROJECT_ROOT="$(dirname "$SRC_ROOT")"
if [ ! -d "$PROJECT_ROOT/build" ]; then
mkdir "$PROJECT_ROOT/build"
fi
BUILD_ROOT="$PROJECT_ROOT/build"
BIN_ROOT="$SRC_ROOT/bin"
CUSTOM_ROOT="$SRC_ROOT/custom"
CUSTOM_BIN="$CUSTOM_ROOT/bin"

# Get the build mode
BUILD_MODE="$1"
if [ -z "$BUILD_MODE" ]; then
    BUILD_MODE="-DDEV_BUILD"
fi

WARNINGS="-Wno-write-strings -Wno-comment"

FLAGS="-D_GNU_SOURCE -fPIC -fpermissive $BUILD_MODE"
INCLUDES="-I$SRC_ROOT -I$CUSTOM_ROOT"

# Execute
g++ $WARNINGS $FLAGS $INCLUDES "$BIN_ROOT/4ed_build.cpp" -g -o "$BUILD_ROOT/build"
pushd "$SRC_ROOT"
"$BUILD_ROOT/build"
popd
