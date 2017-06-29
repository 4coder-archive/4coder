#!/bin/bash

BUILD_MODE="$1"
if [ -z "$BUILD_MODE" ]; then
    BUILD_MODE="-DDEV_BUILD"
fi

chmod 777 detect_os.sh
os=$("./detect_os.sh")

if [[ "$os" == "linux" ]]; then
WARNINGS="-Wno-write-strings -Wno-comment "
elif [[ "$os" == "mac" ]]; then
WARNINGS="-Wno-write-strings -Wno-comment -Wno-logical-op-parentheses -Wno-null-dereference -Wno-switch"
fi

FLAGS="-D_GNU_SOURCE -fPIC -fpermissive $BUILD_MODE"

g++ $WARNINGS $FLAGS meta/build.cpp -g -o ../build/build
../build/build



