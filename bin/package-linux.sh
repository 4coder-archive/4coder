#!/bin/bash

chmod +x bin/build-linux.sh
bin/build-linux.sh "-DPACKAGE_DEMO_X64"
bin/build-linux.sh "-DPACKAGE_SUPER_X64"
