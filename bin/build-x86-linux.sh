#!/bin/bash

ME="$(readlink -f "$0")"
LOCATION="$(dirname "$ME")"
$LOCATION/build-linux.sh -DDEV_BUILD_X86


