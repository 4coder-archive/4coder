#!/bin/bash

# If any command errors, stop the script
set -e

# Set up directories
ORIGINAL=$PWD
ME="$(readlink -f "$0")"
LOCATION="$(dirname "$ME")"
cd $LOCATION
cd ..
CUSTOM_ROOT=$PWD
cd $ORIGINAL

target=$1
if [ -z "$target" ]
then
 echo error: no input file
 exit 1
fi

full_target=$target
if [[ ${target:0:1} != "/" ]];
then
full_target="$PWD/$target"
fi

dst=$2
if [[ $dst == "" ]];
then
dst=.
fi

debug=-g

opts="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-missing-declarations -Wno-logical-op-parentheses -g -DOS_LINUX=1 -DOS_WINDOWS=0 -DOS_MAC=0"

pushd $dst
g++ -I"$CUSTOM_ROOT" $opts $full_target -o one_time
popd


