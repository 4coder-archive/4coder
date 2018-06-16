#!/bin/sh

code_home="$PWD"
opts="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-writable-strings -g"

cd ../build > /dev/null
preproc_file=4coder_command_metadata.i
meta_macros="-DCUSTOM_COMMAND_SIG=CUSTOM_COMMAND_SIG -DCUSTOM_DOC=CUSTOM_DOC -DCUSTOM_ALIAS=CUSTOM_ALIAS -DNO_COMMAND_METADATA"
g++ -I"$code_home" $meta_macros $opts -std=gnu++0x "$SOURCE" -E -o $preproc_file
g++ -I"$code_home" $opts -std=gnu++0x "$code_home/4coder_metadata_generator.cpp" -o metadata_generator
metadata_generator -R "$code_home" "$PWD/$preproc_file"
cd $code_home > /dev/null

