#include "constants.h"
#include "ui.h"
#include "synth.h"
#include <glad/glad.h>
#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>
#include <math.h>
#include <assert.h>

#define DEFAULT_FONT "../assets/fonts/Lato-Regular.ttf"

constexpr NVGcolor RGBAtoColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (NVGcolor){
        .r = (float)r / 255.0f,
        .g = (float)g / 255.0f,
        .b = (float)b / 255.0f,
        .a = (float)a / 255.0f
    };
}

constexpr float DegToRad(float degrees) {
    return degrees / 180.f * (float)M_PI;
}

void ClearBackground(NVGcolor color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

static constexpr NVGcolor BG_GREY = RGBAtoColor(39,42,45,255);
static constexpr NVGcolor OSC_ENABLED_GREY = RGBAtoColor(77,79,82,255);
static constexpr NVGcolor KNOB_BG = BG_GREY;
static constexpr NVGcolor KNOB_ACTIVE_PURPLE = RGBAtoColor(164,137,248,255);
static constexpr NVGcolor KNOB_INACTIVE_GREY = OSC_ENABLED_GREY;
static constexpr NVGcolor KNOB_LABEL_BG = RGBAtoColor(63,66,69,255);
static constexpr NVGcolor DARK_GREY = RGBAtoColor(25, 25, 25, 255);
static constexpr NVGcolor LIME_GREEN = RGBAtoColor(50, 205, 50, 255);
static constexpr NVGcolor ALMOST_WHITE = RGBAtoColor(214, 214, 214, 255);
static constexpr NVGcolor WHITE = RGBAtoColor(255, 255, 255, 255);

bool UI::init(Synth* synth) {
    _synth = synth;
    int flags = NVG_STENCIL_STROKES | NVG_ANTIALIAS;
    _nvg = nvgCreateGL3(flags);
    if (NULL == _nvg) {
        SDL_Log("Failed to create NVG Context");
        return false;
    }
    _fontId = nvgCreateFont(_nvg, "default", DEFAULT_FONT);
    if (_fontId < 0) {
        SDL_Log("Failed to load font: %s", DEFAULT_FONT);
        return false;
    }
    nvgFontFaceId(_nvg, _fontId);
    return true;
}

void UI::drawFilledCircle(float centerX, float centerY, float radius, NVGcolor color) {
    nvgBeginPath(_nvg);
    nvgCircle(_nvg, centerX, centerY, radius);
    nvgFillColor(_nvg, color);
    nvgFill(_nvg);
}

void UI::drawArc(float cx, float cy, float radius, float startDeg, float endDeg, float strokePx, NVGcolor color) {
    nvgBeginPath(_nvg);
    nvgArc(_nvg, cx, cy, radius, DegToRad(startDeg), DegToRad(endDeg), NVG_CW);
    nvgStrokeWidth(_nvg, strokePx);
    nvgStrokeColor(_nvg, color);
    nvgStroke(_nvg);
}

void UI::drawLine(float x1, float y1, float x2, float y2, float strokeWidthPx, NVGcolor color) {
    nvgBeginPath(_nvg);
    nvgMoveTo(_nvg, x1, y1);
    nvgLineTo(_nvg, x2, y2);
    nvgStrokeWidth(_nvg, strokeWidthPx);
    nvgStrokeColor(_nvg, color);
    nvgStroke(_nvg);
}

void UI::drawLabel(const char* text, float x, float y, NVGcolor bgColor, NVGcolor fgColor, std::optional<float> width, std::optional<float> height) {
    nvgBeginPath(_nvg);

    // Get bounds of rendered text
    float bounds[4] = {};
    nvgFontSize(_nvg, 12);
    nvgTextBounds(_nvg, 0, 0, text, NULL, bounds);
    float textWidth = bounds[2] - bounds[0];
    float textHeight = bounds[3] - bounds[1];

    // Determine dimensions of rounded rectangle containing text
    float rh = (height ? height.value() : (2.f * textHeight));
    float rw = (width ? width.value() : (rh + textWidth));
    float radius = rh / 2.f;

    nvgRoundedRect(_nvg, x, y, rw, rh, radius);
    nvgFillColor(_nvg, bgColor);
    nvgFill(_nvg);

    // Position text in middle of rounded rect
    nvgTextAlign(_nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
    nvgFillColor(_nvg, fgColor);
    nvgText(_nvg, x + rw/2.f, y + rh/2.f, text, NULL);
}

void UI::drawKnob(float x, float y, float level) {
    const float r = 25.f; // radius
    const float stroke = 3.f;
    const float startDeg = 120.f;
    const float endDeg = 420.f;
    const float levelDeg = startDeg + (endDeg - startDeg) * level;

    float cx = x + r; // center of circle
    float cy = y + r;

    drawFilledCircle(cx, cy, r, KNOB_BG);
    drawArc(cx, cy, r - 1.5f*stroke, startDeg, levelDeg, stroke, KNOB_ACTIVE_PURPLE);
    drawArc(cx, cy, r - 1.5f*stroke, levelDeg, endDeg, stroke, KNOB_INACTIVE_GREY);

    // Draw line for knob level indicator
    float outerRadius = r - 1.5f*stroke;
    nvgSave(_nvg);
    nvgTranslate(_nvg, cx, cy);
    nvgRotate(_nvg, DegToRad(levelDeg));
    drawLine(.3f * outerRadius, 0, outerRadius, 0, stroke, WHITE);
    nvgRestore(_nvg);

    drawLabel("LEVEL", x, y + 2.f*r + 8, KNOB_LABEL_BG, WHITE, 2.f*r, 20);
}

void UI::drawWaveform() {
}

void UI::draw() {
    ClearBackground(OSC_ENABLED_GREY);

    nvgBeginFrame(_nvg, WINDOW_WIDTH, WINDOW_HEIGHT, 1.f);

    drawKnob(100.f, 100.f, .7f);

    nvgEndFrame(_nvg);
}
