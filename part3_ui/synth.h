#pragma once

#include <SDL.h>

#define DEFAULT_FREQ 220.0

typedef double (*OscillatorFn)(double t, double freqHz);
double sine(double t, double freqHz);
double square(double t, double freqHz);
double triangle(double t, double freqHz);
double saw(double t, double freqHz);
double whitenoise(double t, double freqHz);

struct Synth {
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_AudioDeviceID audioDevice;
    bool loopShouldStop = false;
    bool start = false;
    bool stop = false;
    bool soundEnabled = false;
    double freqHz = DEFAULT_FREQ;
    OscillatorFn oscFn = sine;
};

extern Synth gSynth;
