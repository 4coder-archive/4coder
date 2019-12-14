#!/bin/bash

# If any command errors, stop the script
set -e

# Store the real CWD
ME="$(readlink -f "$0")"
LOCATION="$(dirname "$ME")"
CODE_HOME="$(dirname "$LOCATION")"

# Find the most reasonable candidate build file
SOURCE="$1"
if [ -z "$SOURCE" ]; then
    SOURCE="$(readlink -f "$CODE_HOME/4coder_default_bindings.cpp")"
fi

opts="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-writable-strings -g"
arch=-m64

preproc_file=4coder_command_metadata.i
meta_macros="-DMETA_PASS"
g++ -I"$CODE_HOME" $meta_macros $arch $opts $debug -std=gnu++0x "$SOURCE" -E -o $preproc_file
g++ -I"$CODE_HOME" $opts $debug -std=gnu++0x "$CODE_HOME/4coder_metadata_generator.cpp" -o "$CODE_HOME/metadata_generator"
"$CODE_HOME/metadata_generator" -R "$CODE_HOME" "$PWD/$preproc_file"

g++ -I"$CODE_HOME" $arch $opts $debug -std=gnu++0x "$SOURCE" -shared -o custom_4coder.so -fPIC

rm "$CODE_HOME/metadata_generator"
rm $preproc_file
