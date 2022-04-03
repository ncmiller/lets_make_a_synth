#pragma once

#include "sdlwrapper.h"
#include "oscillator.h"
#include "ui.h"
#include "constants.h"

struct Synth {
    bool start = false;
    bool stop = false;
    bool shouldQuit = false;
    SDLWrapper sdl;
    Oscillator osc;
    UI ui;
};
