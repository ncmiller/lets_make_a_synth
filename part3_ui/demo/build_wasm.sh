#!/bin/bash

MY_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $MY_DIR
cp -r ../../assets .
emcc \
    -v \
    -O2 \
    -DIS_WASM_BUILD \
    -s MAX_WEBGL_VERSION=2 \
    -s GL_UNSAFE_OPTS=1 \
    -s USE_SDL=2 \
    --preload-file assets \
    -I ../../third_party/glad/include \
    ../../third_party/glad/src/glad.c \
    -I ../../third_party/nanovg \
    ../../third_party/nanovg/nanovg.c \
    -I .. \
    -x c++ \
    -std=c++17 \
    ../main.cpp \
    ../audio.cpp \
    ../oscillator.cpp \
    ../sdlwrapper.cpp \
    ../ui.cpp \
    ../utility.cpp \
    ../input.cpp \
    -o synth.js
