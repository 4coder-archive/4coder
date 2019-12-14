#!/bin/bash

os="unknown"
if [[ "$OSTYPE" == "darwin"* ]]; then
echo "mac"
elif [[ "$OSTYPE" == "linux-gnu" ]]; then
echo "linux"
fi

