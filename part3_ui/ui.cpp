#include "constants.h"
#include "ui.h"
#include <SDL2_gfxPrimitives.h>

constexpr uint32_t color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // 0xAABBGGRR
    return (uint32_t)((a << 24) | (b << 16) | (g << 8) | (r << 0));
}

constexpr SDL_Color sdlColor(uint32_t color32) {
    return {
        .r = (uint8_t)((color32 >> 0) & 0xFF),
        .g = (uint8_t)((color32 >> 8) & 0xFF),
        .b = (uint8_t)((color32 >> 16) & 0xFF),
        .a = (uint8_t)((color32 >> 24) & 0xFF),
    };
}

constexpr uint32_t BG_GREY = color32(39,42,45,255);
constexpr uint32_t OSC_ENABLED_GREY = color32(77,79,82,255);
constexpr uint32_t KNOB_BG = BG_GREY;
constexpr uint32_t KNOB_ACTIVE_PURPLE = color32(164,137,248,255);
constexpr uint32_t KNOB_INACTIVE_GREY = OSC_ENABLED_GREY;
constexpr uint32_t KNOB_LABEL_BG = color32(63,66,69,255);
constexpr uint32_t DARK_GREY = color32(25, 25, 25, 255);
constexpr uint32_t LIME_GREEN = color32(50, 205, 50, 255);
constexpr uint32_t ALMOST_WHITE = color32(214, 214, 214, 255);

void UI::init(Synth* synth) {
    _synth = synth;
    _renderer = _synth->renderer;
    _font = TTF_OpenFont("../assets/fonts/Lato-Regular.ttf", _fontSize);
}

void UI::setDrawColor(uint32_t c32) {
    SDL_Color color = sdlColor(c32);
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
}

void UI::drawWaveform() {
    int16_t padding = (int)(0.1 * (double)WINDOW_WIDTH);
    int16_t drawingWidth = WINDOW_WIDTH - 2 * padding;
    int16_t drawingHeight = WINDOW_HEIGHT - 2 * padding;
    const double periodS = 1.0 / DEFAULT_FREQ;
    int16_t lastx = -1;
    int16_t lasty = -1;
    for (double t = 0.0; t < periodS; t += dt) {
        double y = _synth->osc->getSample(t, _synth->freqHz);
        // Convert (t,y) to 2-D coord (px,py)
        int16_t px = padding + (int16_t)((t / periodS) * drawingWidth);
        int16_t py = padding + (int16_t)((drawingHeight / 2) * (1.0 - y));
        uint8_t widthPx = (_synth->soundEnabled ? 4 : 1);
        if (lastx != -1) {
            thickLineColor(_renderer, lastx, lasty, px, py, widthPx, LIME_GREEN);
        }
        lastx = px;
        lasty = py;
    }
}

void UI::drawFilledCircle(int16_t centerX, int16_t centerY, int16_t radius, uint32_t color) {
    filledCircleColor(_renderer, (int16_t)centerX, (int16_t)centerY, (int16_t)radius, color);
    aaellipseColor(_renderer, (int16_t)centerX, (int16_t)centerY, (int16_t)radius+1, (int16_t)radius, color);
    aacircleColor(_renderer, (int16_t)centerX, (int16_t)centerY, (int16_t)radius, color);
}

void UI::drawText(const char* text, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Blended(_font, text, sdlColor(ALMOST_WHITE));
    SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_FreeSurface(surface);

    int w = 0;
    int h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderCopy(_renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
}

void UI::drawArc(
        int16_t centerX,
        int16_t centerY,
        int16_t radius,
        int16_t strokeWidth,
        int16_t startAngleDeg,
        int16_t endAngleDeg,
        uint32_t colorFg,
        uint32_t colorBg) {
    drawFilledCircle(centerX, centerY, radius+strokeWidth/2, colorFg);
    drawFilledCircle(centerX, centerY, radius-strokeWidth/2, colorBg);
    filledPieColor(_renderer, centerX, centerY, radius+strokeWidth+1, startAngleDeg, endAngleDeg, colorBg);
}

void UI::draw() {
    // Set background
    setDrawColor(BG_GREY);
    SDL_RenderClear(_renderer);

    drawText("Hello, World!", 10, 10);
    drawArc(100, 100, 25, 4, 60, 120, KNOB_ACTIVE_PURPLE, KNOB_BG);
    drawWaveform();
}
