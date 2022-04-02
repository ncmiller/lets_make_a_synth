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

static const char* DEFAULT_FONT = "../assets/fonts/Lato-Regular.ttf";
static constexpr float LABEL_HEIGHT = 20.f;
static constexpr float KNOB_WIDTH = 50.f;
static constexpr float KNOB_LABEL_GAP = 8.f;
static constexpr float KNOB_HEIGHT = (KNOB_WIDTH + KNOB_LABEL_GAP + LABEL_HEIGHT);
static constexpr NVGcolor BG_GREY = RGBAtoColor(39,42,45,255);
static constexpr NVGcolor OSC_ENABLED_GREY = RGBAtoColor(77,79,82,255);
static constexpr NVGcolor KNOB_BG = BG_GREY;
static constexpr NVGcolor KNOB_ACTIVE_PURPLE = RGBAtoColor(164,137,248,255);
static constexpr NVGcolor KNOB_INACTIVE_GREY = OSC_ENABLED_GREY;
static constexpr NVGcolor KNOB_LABEL_BG = RGBAtoColor(63,66,69,255);
static constexpr NVGcolor DARK_GREY = RGBAtoColor(25, 25, 25, 255);
static constexpr NVGcolor LIME_GREEN = RGBAtoColor(50, 205, 50, 255);
static constexpr NVGcolor BLACK = RGBAtoColor(0, 0, 0, 255);
static constexpr NVGcolor ALMOST_WHITE = RGBAtoColor(214, 214, 214, 255);
static constexpr NVGcolor WHITE = RGBAtoColor(255, 255, 255, 255);

void ClearBackground(NVGcolor color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

size_t HashCombine(size_t seed, size_t value) {
    return (seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

// Helper class, make it easy to generate and push a new ID on the ID stack
// in the current scope and automatically pop when leaving scope.
template <typename T>
class ScopedId {
public:
    ScopedId(std::vector<size_t>& idStack, const T& data) : _idStack(idStack) {
        size_t seed = idStack.back();
        size_t id = HashCombine(seed, std::hash<T>{}(data));
        _idStack.push_back(id);
    }
    ~ScopedId() {
        _idStack.pop_back();
    }
    size_t value() const {
        return _idStack.back();
    }
private:
    std::vector<size_t>& _idStack;
};

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
    _idStack.push_back(std::hash<const char*>{}("root"));
    return true;
}

bool UI::mouseInRect(float x1, float y1, float x2, float y2) {
    return ((_mouseX >= x1 && _mouseX <= x2) &&
            (_mouseY >= y1 && _mouseY <= y2));
}

bool UI::isActive(size_t id) {
    return (id == _activeId);
}

bool UI::activeExists() {
    return (_activeId != 0);
}

bool UI::isPreactive(size_t id) {
    return (id == _preactiveId);
}

void UI::onControlEvent(SDL_Event event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        _mouseButtonDown = true;
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        _mouseButtonUp = true;
        uint32_t now = SDL_GetTicks();
        uint32_t deltaMs = now - _mouseLastClick;
        if (deltaMs < 250) {
            _mouseDoubleClick = true;
        }
        _mouseLastClick = now;
    } else if (event.type == SDL_MOUSEMOTION) {
        _mouseX = event.motion.x;
        float newMouseY = event.motion.y;
        _mouseYDelta = (newMouseY - _mouseY);
        _mouseY = newMouseY;
    }
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

void UI::label(
        const char* text,
        float x, float y,
        NVGcolor bgColor, NVGcolor fgColor,
        std::optional<float> width, std::optional<float> height) {
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

void UI::knob(const char* text, float x, float y, float zero, float defaultLev, float* level, const char* valuetext) {
    size_t id = ScopedId(_idStack, text).value();

    bool mouseInside = mouseInRect(x, y, x+KNOB_WIDTH, y+KNOB_HEIGHT);
    bool resetToDefault = isActive(id) && _mouseDoubleClick;
    if (!isActive(id) && !isPreactive(id)) {
        if (mouseInside && !activeExists()) {
            _preactiveId = id;
        }
    }
    if (!isActive(id) && isPreactive(id)) {
        if (!mouseInside) {
            _preactiveId = 0;
        } else if (_mouseButtonDown) {
            _activeId = id;
        }
    }
    if (isActive(id)) {
        assert(isPreactive(id));
        if (_mouseButtonUp) {
            _activeId = 0;
        }
    }

    float stroke = 2.5f;
    if (isActive(id) || isPreactive(id)) {
        stroke = 3.f;
    }
    float r = KNOB_WIDTH / 2.f; // radius
    float startDeg = 120.f;
    float endDeg = 420.f;
    float cx = x + r; // center of circle
    float cy = y + r;

    float levelToUse = *level;
    if (resetToDefault) {
        levelToUse = defaultLev;
        *level = levelToUse;
    } else if (isActive(id)) {
        float pxToLevelScalar = 1.f / 150.f;
        float newLevel = levelToUse - pxToLevelScalar * _mouseYDelta;
        newLevel = std::max(-zero, newLevel);
        newLevel = std::min(1.f - zero, newLevel);
        levelToUse = newLevel;
        *level = levelToUse;
    }

    // Normalize level from 0 to 1
    float normalizedLevel = levelToUse + zero;
    assert (normalizedLevel >= -0.0001f && normalizedLevel <= 1.0001f);

    float levelDeg = startDeg + (endDeg - startDeg) * normalizedLevel;
    float zeroDeg = startDeg + (endDeg - startDeg) * zero;

    // Draw knob background and arcs
    drawFilledCircle(cx, cy, r, KNOB_BG);
    if (normalizedLevel < zero) {
        // startDeg -> (level -> zero) -> endDeg
        drawArc(cx, cy, r - 1.5f - stroke, startDeg, levelDeg, stroke, KNOB_INACTIVE_GREY);
        drawArc(cx, cy, r - 1.5f - stroke, levelDeg, zeroDeg, stroke, KNOB_ACTIVE_PURPLE);
        drawArc(cx, cy, r - 1.5f - stroke, zeroDeg, endDeg, stroke, KNOB_INACTIVE_GREY);
    } else {
        // startDeg -> (zero -> level) -> endDeg
        SDL_Log("LevelDeg = %f, ZeroDeg = %f", levelDeg, zeroDeg);
        drawArc(cx, cy, r - 1.5f - stroke, startDeg, zeroDeg, stroke, KNOB_INACTIVE_GREY);
        drawArc(cx, cy, r - 1.5f - stroke, zeroDeg, levelDeg, stroke, KNOB_ACTIVE_PURPLE);
        drawArc(cx, cy, r - 1.5f - stroke, levelDeg, endDeg, stroke, KNOB_INACTIVE_GREY);
    }

    // Draw line for knob level indicator
    float outerRadius = r - 1.5f - stroke/2.f;
    nvgSave(_nvg);
    nvgTranslate(_nvg, cx, cy);
    nvgRotate(_nvg, DegToRad(levelDeg));
    drawLine(.3f * outerRadius, 0, outerRadius, 0, stroke, WHITE);
    nvgRestore(_nvg);

    // Knob label at the bottom
    label(text, x, y + KNOB_WIDTH + KNOB_LABEL_GAP, KNOB_LABEL_BG, WHITE, KNOB_WIDTH, LABEL_HEIGHT);

    // Overlay text of current value. Only visible if preactive or active.
    float overlayOpacity = 0.0f;
    if (isActive(id)) {
        overlayOpacity = 0.6f;
    } else if (isPreactive(id)) {
        overlayOpacity = 0.3f;
    }
    NVGcolor bg = KNOB_LABEL_BG;
    bg.a = overlayOpacity;
    NVGcolor fg = WHITE;
    fg.a = overlayOpacity;
    label(valuetext, cx-r, cy-1.6f*r, bg, fg, std::nullopt, LABEL_HEIGHT * 0.8f);
}

void UI::oscillator(const char* name, float x, float y) {
    size_t id = ScopedId(_idStack, name).value();

    float pad = 10.f; // between background and oscillator widgets
    float num_knobs = 2.f;
    float rw = num_knobs * (pad + KNOB_WIDTH) + pad;
    float rh = 2.f * pad + KNOB_HEIGHT;

    nvgBeginPath(_nvg);

    // Oscillator background
    nvgRoundedRect(_nvg, x, y, rw, rh, 5.f);
    nvgFillColor(_nvg, OSC_ENABLED_GREY);
    nvgStrokeWidth(_nvg, 2.f);
    nvgStrokeColor(_nvg, DARK_GREY);
    nvgFill(_nvg);
    nvgStroke(_nvg);

    // Knobs
    float xoff = x + pad;
    float yoff = y + pad;
    static float levelValue = 0.7f;
    char levelText[16] = {};
    snprintf(levelText, sizeof(levelText), "%3.1f%%", levelValue * 100.f);
    knob("LEVEL", xoff, yoff, 0.f, 0.7f, &levelValue, levelText);

    xoff += (KNOB_WIDTH + pad);
    static float panValue = 0.0f;
    char panText[16] = {};
    snprintf(panText, sizeof(panText), "%3.1f%%", panValue * 100.f);
    knob("PAN", xoff, yoff, 0.5f, 0.0f, &panValue, panText);
}

void UI::draw() {
    ClearBackground(BG_GREY);

    nvgBeginFrame(_nvg, WINDOW_WIDTH, WINDOW_HEIGHT, 1.f);

    oscillator("OSC1", 100.f, 100.f);

    // TODO
    //
    // link knob values with oscillator
    // coarse pitch
    // fine pitch
    // piano roll
    // waveform


    nvgEndFrame(_nvg);

    // Auto-clear mouse events now that we've drawn
    _mouseButtonDown = false;
    _mouseButtonUp = false;
    _mouseYDelta = 0.f;
    _mouseDoubleClick = false;
}
