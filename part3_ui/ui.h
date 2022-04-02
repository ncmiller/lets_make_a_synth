#pragma once

#include <SDL.h>
#include <nanovg.h>
#include <stdint.h>
#include <optional>
#include <vector>

struct Synth;
struct NVGcontext;
struct NVGColor;

class UI {
public:
    bool init(Synth* synth);
    void draw();
    void onControlEvent(SDL_Event event);

private:
    // Primitive drawing
    void drawFilledCircle(float centerX, float centerY, float radius, NVGcolor color);
    void drawLine(float x1, float y1, float x2, float y2, float strokeWidthPx, NVGcolor color);
    void drawArc(float cx, float cy, float radius, float startDeg, float endDeg, float strokePx, NVGcolor color);

    // Synth widgets
    void label(const char* text, float x, float y, NVGcolor bgColor, NVGcolor fgColor, std::optional<float> width = std::nullopt, std::optional<float> height = std::nullopt);
    void knob(const char* text, float x, float y, float* level);
    void oscillator(const char* name, float x, float y);

    // Utility functions
    bool mouseInRect(float x1, float y1, float x2, float y2);
    bool activeExists();
    bool isActive(size_t id);
    bool isPreactive(size_t id);

    Synth* _synth = nullptr; // parent object
    NVGcontext* _nvg = nullptr;
    int _fontId = 0;
    std::vector<size_t> _idStack;
    size_t _preactiveId = 0; // ID of widget about to be active (e.g. hovering)
    size_t _activeId = 0; // ID of widget that is active, being interacted with (e.g. mouse click)
    float _mouseX = 0.f;
    float _mouseY = 0.f;
    bool _mouseButtonDown = false;
    bool _mouseButtonUp = false;
    float _mouseYDelta = 0.f;
};

