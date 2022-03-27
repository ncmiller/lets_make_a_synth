#pragma once

#include <stdint.h>
#include <nanovg.h>
#include <optional>

struct Synth;
struct NVGcontext;
struct NVGColor;

class UI {
public:
    bool init(Synth* synth);
    void draw();

private:
    // Primitive drawing
    void drawFilledCircle(float centerX, float centerY, float radius, NVGcolor color);
    void drawLine(float x1, float y1, float x2, float y2, float strokeWidthPx, NVGcolor color);
    void drawArc(float cx, float cy, float radius, float startDeg, float endDeg, float strokePx, NVGcolor color);

    // Synth widgets
    void drawLabel(const char* text, float x, float y, NVGcolor bgColor, NVGcolor fgColor, std::optional<float> width = std::nullopt, std::optional<float> height = std::nullopt);
    void drawKnob(const char* text, float x, float y, float level);
    void drawOscillator(float x, float y);
    void drawWaveform();

    Synth* _synth = nullptr;
    NVGcontext* _nvg = nullptr;
    int _fontId = 0;
};

