#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include "synth.h"
#include "oscillator.h"

#define DEFAULT_TEXT_FONT_SIZE 16

class UI {
public:
    void init(Synth* synth);
    void draw();

private:
    void drawFilledCircle(int centerX, int centerY, int radius);
    void drawWaveform();
    void drawText(const char* text, int x, int y);
    void drawArc(
        int centerX,
        int centerY,
        int radius,
        int strokeWidth,
        double startAngleRad, // clockwise
        double endAngleRad);

    Synth* _synth;
    SDL_Renderer* _renderer;
    TTF_Font* _font = nullptr;
    int _fontSize = DEFAULT_TEXT_FONT_SIZE;
};

