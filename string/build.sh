#!/bin/bash

WARNINGS="-Wno-write-strings -Wno-comment -Wno-logical-op-parentheses -Wno-null-dereference -Wno-switch"
FLAGS="-D_GNU_SOURCE -fPIC -fpermissive"

g++ $WARNINGS $FLAGS ../code/string/string_builder.cpp -g -o ../build/string_builder

pushd string
../../build/string_builder
popd

