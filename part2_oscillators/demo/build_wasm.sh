#!/bin/bash

MY_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $MY_DIR
emcc \
    -v \
    -O3 \
    -DIS_WASM_BUILD \
    -s USE_SDL=2 \
    ../main.cpp \
    -o synth.js
