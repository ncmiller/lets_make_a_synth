#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include "oscillator.h"

#define DEFAULT_TEXT_FONT_SIZE 16

class UI {
public:
    void init(SDL_Renderer* renderer);
    void draw(const Oscillator& osc, double freqHz, bool isPlaying);

private:
    void drawCircle(int centerX, int centerY, int radius);
    void drawWaveform(const Oscillator& osc, double freqHz, bool isPlaying);
    void drawText(const char* text, int x, int y);

    SDL_Renderer* _renderer = nullptr;
    TTF_Font* _font = nullptr;
    int _fontSize = DEFAULT_TEXT_FONT_SIZE;
};

