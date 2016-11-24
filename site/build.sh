#!/bin/bash

WARNINGS="-Wno-write-strings"
FLAGS="-D_GNU_SOURCE -fPIC -fpermissive -DSITE_BUILD"

pushd ..
BASEDIR="$PWD"
g++ $WARNINGS $FLAGS $BASEDIR/build.cpp -g -o ../build/build
../build/build
popd
