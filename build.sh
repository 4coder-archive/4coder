#!/bin/bash

BUILD_MODE="$1"
if [ -z "$BUILD_MODE" ]; then
    BUILD_MODE="-DDEV_BUILD"
fi

WARNINGS="-Wno-write-strings"
FLAGS="-D_GNU_SOURCE -fPIC -fpermissive $BUILD_MODE"

g++ $WARNINGS $FLAGS meta/build.cpp -g -o ../build/build
../build/build



