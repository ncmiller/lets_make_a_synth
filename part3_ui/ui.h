#pragma once

#include <SDL.h>
#include "oscillator.h"

class UI {
public:
    void init(SDL_Renderer* renderer) { _renderer = renderer; }
    void drawCircle(int centerX, int centerY, int radius);
    void drawWaveform(const Oscillator& osc, double freqHz, bool isPlaying);

private:
    SDL_Renderer* _renderer = nullptr;
};

