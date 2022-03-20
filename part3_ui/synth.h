#pragma once

#include "ui.h"
#include "oscillator.h"
#include <SDL.h>

#define DEFAULT_FREQ 220.0

struct Synth {
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_AudioDeviceID audioDevice;
    bool loopShouldStop = false;
    bool start = false;
    bool stop = false;
    bool soundEnabled = false;
    double freqHz = DEFAULT_FREQ;
    Oscillator osc;
    UI ui;
};

extern Synth gSynth;
