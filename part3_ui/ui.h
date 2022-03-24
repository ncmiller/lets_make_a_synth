#pragma once

#include <stdint.h>

struct Synth;
struct NVGcontext;

#define DEFAULT_TEXT_FONT_SIZE 16

class UI {
public:
    bool init(Synth* synth);
    void draw();

private:
    void setDrawColor(uint32_t color);
    void drawFilledCircle(int16_t centerX, int16_t centerY, int16_t radius, uint32_t color);
    void drawWaveform();
    void drawText(const char* text, int x, int y);
    void drawArc(
        int16_t centerX,
        int16_t centerY,
        int16_t radius,
        int16_t strokeWidth,
        int16_t startAngleDeg, // counter-clockwise, starting at x=1,y=0
        int16_t endAngleDeg,
        uint32_t colorFg,
        uint32_t colorBg);

    Synth* _synth = nullptr;
    NVGcontext* _nvgContext = nullptr;
};

