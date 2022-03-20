#pragma once

#include <SDL.h>
#include <memory>

class Oscillator;
class UI;

struct Synth {
    Synth();
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_AudioDeviceID audioDevice;
    bool loopShouldStop = false;
    bool start = false;
    bool stop = false;
    bool soundEnabled = false;
    double freqHz = DEFAULT_FREQ;
    std::unique_ptr<Oscillator> osc;
    std::unique_ptr<UI> ui;
};
