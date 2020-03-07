#!/bin/bash

chmod +x bin/build-mac.sh
bin/build-mac.sh "-DPACKAGE_DEMO_X64"
bin/build-mac.sh "-DPACKAGE_SUPER_X64"
