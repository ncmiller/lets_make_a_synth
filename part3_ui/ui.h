#pragma once

#include <SDL.h>
#include "oscillator.h"

namespace UI {

void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius);
void drawWaveform(SDL_Renderer* renderer, Oscillator::Fn oscFn, double freqHz, bool isPlaying);

} // namespace UI

