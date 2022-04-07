#pragma once

#include "sdlwrapper.h"
#include "oscillator.h"
#include "ui.h"
#include "input.h"
#include "constants.h"

struct Synth {
    bool running = true;
    SDLWrapper sdl;
    Input input;
    Oscillator osc;
    UI ui;
};
