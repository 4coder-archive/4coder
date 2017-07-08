#!/bin/bash

WARNINGS="-Wno-write-strings"
FLAGS="-D_GNU_SOURCE -fPIC -fpermissive -DPACKAGE"

g++ $WARNINGS $FLAGS meta/build.cpp -g -o ../build/build
../build/build
