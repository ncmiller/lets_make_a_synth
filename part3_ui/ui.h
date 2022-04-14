#pragma once

#include <SDL.h>
#include <nanovg.h>
#include <stdint.h>
#include <optional>
#include <vector>
#include <array>

struct Synth;
class Input;
struct NVGcontext;
struct NVGColor;

class UI {
public:
    bool Init(Synth* synth);
    void Draw();

private:
    // Primitive drawing
    void DrawFilledCircle(float centerX, float centerY, float radius, NVGcolor color);
    void DrawLine(float x1, float y1, float x2, float y2, float strokeWidthPx, NVGcolor color);
    void DrawArc(float cx, float cy, float radius, float startDeg, float endDeg, float strokePx, NVGcolor color);

    // Synth widgets
    void Label(
            const char* text,
            float x, float y,
            float fontsize,
            NVGcolor color,
            int alignFlags = NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    void RoundRectLabel(
            const char* text,
            float x, float y, // center of circular button
            float fontsize,
            NVGcolor bgColor, NVGcolor fgColor,
            std::optional<float> width = std::nullopt, std::optional<float> height = std::nullopt);
    bool ArrowButton(
            float x, float y, // center of button
            float radius,
            bool isLeft);
    void Knob(
            const char* text,
            float x, float y,
            float zero, // zero point of knob, [0.0, 1.0]
            float defaultLev, // level to use on double-click, default level, relative to zero
            float* level, // current level of knob, relative to zero, range [-zero, 1-zero]
            const char* valuetext);
    void Oscillator(const char* name, float x, float y);

    // Utility functions
    bool MouseInRect(float x1, float y1, float x2, float y2);
    bool MouseInCircle(float x, float y, float radius);
    void UpdateOscillatorVisualization();
    bool ActiveExists();
    bool IsActive(size_t id);
    bool IsPreactive(size_t id);

    Synth* _synth = nullptr; // parent object
    Input* _input = nullptr;
    NVGcontext* _nvg = nullptr;
    int _fontId = 0;

    // ID stack stuff
    std::vector<size_t> _idStack;
    size_t _preactiveId = 0; // ID of widget about to be active (e.g. hovering)
    size_t _activeId = 0; // ID of widget that is active, being interacted with (e.g. mouse click)

    // Cached visualization of selected oscillator
    std::array<float, 256> _oscPoints = {};
};

