#!/bin/bash

WARNINGS="-Wno-write-strings"
FLAGS="-D_GNU_SOURCE -fPIC -fpermissive -DSITE_BUILD"

BASEDIR="$PWD"
g++ $WARNINGS $FLAGS $BASEDIR/meta/build.cpp -g -o ../build/build
../build/build


