#!/bin/sh

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
code_home=$(dirname "$SCRIPT_FILE")

# Find the most reasonable candidate build file
SOURCE="$1"
if [ -z "$SOURCE" ]; then
    SOURCE="$code_home/4coder_default_bindings.cpp"
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

opts="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-writable-strings -g"

cd ../build > /dev/null
preproc_file=4coder_command_metadata.i
meta_macros="-DMETA_PASS"
g++ -I"$code_home" $meta_macros $opts -std=gnu++0x "$SOURCE" -E -o $preproc_file
g++ -I"$code_home" $opts -std=gnu++0x ../code/4coder_metadata_generator.cpp -o metadata_generator
./metadata_generator -R "$code_home" "$PWD/$preproc_file"
cd $code_home > /dev/null

