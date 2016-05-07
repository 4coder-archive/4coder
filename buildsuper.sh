#!/bin/bash
#!/bin/sh

SOURCE="$1"
if [ -z "$SOURCE" ]
then
 SOURCE="4coder_default_bindings.cpp"
fi

g++ -Wno-write-strings -std=gnu++0x $SOURCE -shared -o 4coder_custom.so -fPIC

