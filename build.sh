#!/bin/bash

# Store the real CWD
REAL_PWD="$PWD"

# Find the code home folder
TARGET_FILE="$0"
cd `dirname $TARGET_FILE`
TARGET_FILE=`basename $TARGET_FILE`
while [ -L "$TARGET_FILE" ]
do
    TARGET_FILE=`readlink $TARGET_FILE`
    cd `dirname $TARGET_FILE`
    TARGET_FILE=`basename $TARGET_FILE`
done
PHYS_DIR=`pwd -P`
SCRIPT_FILE=$PHYS_DIR/$TARGET_FILE
CODE_HOME=$(dirname "$SCRIPT_FILE")

# Restore the PWD
cd "$REAL_PWD"

# Get the build mode
BUILD_MODE="$1"
if [ -z "$BUILD_MODE" ]; then
    BUILD_MODE="-DDEV_BUILD"
fi

# Get the OS specific flags
chmod 777 detect_os.sh
os=$("./detect_os.sh")

if [[ "$os" == "linux" ]]; then
WARNINGS="-Wno-write-strings -Wno-comment "
elif [[ "$os" == "mac" ]]; then
WARNINGS="-Wno-write-strings -Wno-comment -Wno-logical-op-parentheses -Wno-null-dereference -Wno-switch"
fi

FLAGS="-D_GNU_SOURCE -fPIC -fpermissive $BUILD_MODE"

# Execute
g++ $WARNINGS $FLAGS $CODE_HOME/meta/build.cpp -g -o ../build/build
../build/build



