#!/bin/sh

FLAGS="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-writable-strings"

pushd ../build > /dev/null
g++ $FLAGS -std=gnu++0x ../code/4coder_metadata_generator.cpp -o metadata_generator
popd > /dev/null

CODE_HOME="$PWD"
../build/metadata_generator -R "$CODE_HOME"
