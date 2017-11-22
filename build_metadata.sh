#!/bin/sh

FLAGS="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-writable-strings"

CODE_HOME="$PWD"

cd ../build > /dev/null
g++ $FLAGS -std=gnu++0x $CODE_HOME/4coder_metadata_generator.cpp -o metadata_generator
cd $CODE_HOME > /dev/null

../build/metadata_generator -R "$CODE_HOME" "$CODE_HOME"
