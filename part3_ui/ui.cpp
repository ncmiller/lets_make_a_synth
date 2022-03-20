#include "constants.h"
#include "ui.h"

static const SDL_Color WHITE = {255, 255, 255, 255};

void UI::init(Synth* synth) {
    _synth = synth;
    _renderer = _synth->renderer;
    _font = TTF_OpenFont("../assets/fonts/Lato-Light.ttf", _fontSize);
}

void UI::drawWaveform() {
    SDL_SetRenderDrawColor(_renderer, 50, 205, 50, 255); // lime green

    int padding = (int)(0.1 * (double)WINDOW_WIDTH);
    int drawingWidth = WINDOW_WIDTH - 2 * padding;

    // Draw the waveform from left to right, as if starting
    // from the waveform point at t = 0.
    const double periodS = 1.0 / DEFAULT_FREQ;
    for (double t = 0.0; t < periodS; t += dt) {
        double y = _synth->osc->getSample(t, _synth->freqHz);
        // Convert (t,y) to 2-D coord (px,py)
        int px = padding + (int)((t / periodS) * drawingWidth);
        int py = padding + (int)((drawingWidth / 2) * (1.0 - y));
        int radius = (_synth->soundEnabled ? 4 : 1);
        drawCircle(px, py, radius);
    }
}

void UI::drawCircle(int centerX, int centerY, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(_renderer, centerX + x, centerY + y);
            }
        }
    }
}

void UI::drawText(const char* text, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Blended(_font, text, WHITE);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_FreeSurface(surface);

    int w = 0;
    int h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderCopy(_renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
}

void UI::draw() {
    // Set background
    SDL_SetRenderDrawColor(_renderer, 25, 25, 25, 255);
    SDL_RenderClear(_renderer);

    drawText("Hello, World!", 10, 10);
    drawWaveform();
}
