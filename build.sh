#!/bin/bash

WARNINGS="-Wno-write-strings"
FLAGS="-D_GNU_SOURCE -fPIC -fpermissive -DDEV_BUILD"

g++ $WARNINGS $FLAGS meta/build.cpp -g -o ../build/build
../build/build



