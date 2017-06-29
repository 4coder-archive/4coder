#!/bin/bash

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

# Find the most reasonable candidate build file
SOURCE="$1"
if [ -z "$SOURCE" ]; then
    SOURCE="$CODE_HOME/4coder_default_bindings.cpp"
fi

TARGET_FILE="$SOURCE"
cd `dirname $TARGET_FILE`
TARGET_FILE=`basename $TARGET_FILE`
while [ -L "$TARGET_FILE" ]
do
    TARGET_FILE=`readlink $TARGET_FILE`
    cd `dirname $TARGET_FILE`
    TARGET_FILE=`basename $TARGET_FILE`
done
PHYS_DIR=`pwd -P`
SOURCE=$PHYS_DIR/$TARGET_FILE

# Detect the OS and choose appropriate flags
chmod 777 "$CODE_HOME/detect_os.sh"
os=$("$CODE_HOME/detect_os.sh")
echo "Building on $os"

if [[ "$os" == "linux" ]]; then
FLAGS="-Wno-write-strings"
elif [[ "$os" == "mac" ]]; then
FLAGS="-Wno-null-dereference -Wno-comment -Wno-switch -Wno-writable-strings"
fi

echo "Building custom_4coders.so from $SOURCE"
g++ -I"$CODE_HOME" $FLAGS -std=gnu++0x "$SOURCE" -shared -o custom_4coder.so -fPIC

