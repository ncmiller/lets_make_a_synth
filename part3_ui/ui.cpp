#include "constants.h"
#include "ui.h"
#include "utility.h"
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
static constexpr float PAD = 10.f;
static constexpr float LABEL_HEIGHT = 20.f;
static constexpr float KNOB_WIDTH = 50.f;
static constexpr float KNOB_LABEL_GAP = 8.f;
static constexpr float KNOB_HEIGHT = (KNOB_WIDTH + KNOB_LABEL_GAP + LABEL_HEIGHT);
static constexpr float WAVEFORM_HEIGHT = 2.f * KNOB_HEIGHT + PAD;
static constexpr float WAVEFORM_WIDTH = WAVEFORM_HEIGHT;

static constexpr NVGcolor ALMOST_WHITE = RGBAtoColor(240, 240, 240, 255);
static constexpr NVGcolor BG_GREY = RGBAtoColor(39,42,45,255);
static constexpr NVGcolor OSC_ENABLED_GREY = RGBAtoColor(77,79,82,255);
static constexpr NVGcolor KNOB_BG = BG_GREY;
static constexpr NVGcolor KNOB_ACTIVE_PURPLE = RGBAtoColor(164,137,248,255);
static constexpr NVGcolor KNOB_INACTIVE_GREY = OSC_ENABLED_GREY;
static constexpr NVGcolor KNOB_LABEL_BG = RGBAtoColor(63,66,69,255);
static constexpr NVGcolor DARK_GREY = RGBAtoColor(25, 25, 25, 255);
static constexpr NVGcolor WHITE = RGBAtoColor(255, 255, 255, 255);
static constexpr NVGcolor TRANSPARENT = RGBAtoColor(0, 0, 0, 0);

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

bool UI::Init(Synth* synth) {
    _synth = synth;
    _input = &_synth->input;
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

bool UI::MouseInRect(float x1, float y1, float x2, float y2) {
    return ((_input->mouseX >= x1 && _input->mouseX <= x2) &&
            (_input->mouseY >= y1 && _input->mouseY <= y2));
}

bool UI::IsActive(size_t id) {
    return (id == _activeId);
}

bool UI::ActiveExists() {
    return (_activeId != 0);
}

bool UI::IsPreactive(size_t id) {
    return (id == _preactiveId);
}

void UI::DrawFilledCircle(float centerX, float centerY, float radius, NVGcolor color) {
    nvgBeginPath(_nvg);
    nvgCircle(_nvg, centerX, centerY, radius);
    nvgFillColor(_nvg, color);
    nvgFill(_nvg);
}

void UI::DrawArc(float cx, float cy, float radius, float startDeg, float endDeg, float strokePx, NVGcolor color) {
    nvgBeginPath(_nvg);
    nvgArc(_nvg, cx, cy, radius, DegToRad(startDeg), DegToRad(endDeg), NVG_CW);
    nvgStrokeWidth(_nvg, strokePx);
    nvgStrokeColor(_nvg, color);
    nvgStroke(_nvg);
}

void UI::DrawLine(float x1, float y1, float x2, float y2, float strokeWidthPx, NVGcolor color) {
    nvgBeginPath(_nvg);
    nvgMoveTo(_nvg, x1, y1);
    nvgLineTo(_nvg, x2, y2);
    nvgStrokeWidth(_nvg, strokeWidthPx);
    nvgStrokeColor(_nvg, color);
    nvgStroke(_nvg);
}

// Align text horiz center at x, text vert top at y
void UI::Label(
        const char* text,
        float x, float y,
        float fontsize,
        NVGcolor bgColor, NVGcolor fgColor,
        int alignFlags) {
    nvgBeginPath(_nvg);

    // Get bounds of rendered text
    float bounds[4] = {};
    nvgFontSize(_nvg, fontsize);
    nvgTextBounds(_nvg, 0, 0, text, NULL, bounds);
    float textWidth = bounds[2] - bounds[0];
    float textHeight = bounds[3] - bounds[1];

    // Position text
    nvgTextAlign(_nvg, alignFlags);
    nvgFillColor(_nvg, fgColor);
    nvgText(_nvg, x, y, text, NULL);
}

void UI::RoundRectLabel(
        const char* text,
        float x, float y,
        float fontsize,
        NVGcolor bgColor, NVGcolor fgColor,
        std::optional<float> width, std::optional<float> height) {
    nvgBeginPath(_nvg);

    // Get bounds of rendered text
    float bounds[4] = {};
    nvgFontSize(_nvg, fontsize);
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

void UI::Knob(const char* text, float x, float y, float zero, float defaultLev, float* level, const char* valuetext) {
    size_t id = ScopedId(_idStack, text).value();

    bool mouseInside = MouseInRect(x, y, x+KNOB_WIDTH, y+KNOB_HEIGHT);
    bool resetToDefault = IsActive(id) && _input->mouseDoubleClick;
    if (!IsActive(id) && !IsPreactive(id)) {
        if (mouseInside && !ActiveExists()) {
            _preactiveId = id;
        }
    }
    if (!IsActive(id) && IsPreactive(id)) {
        if (!mouseInside) {
            _preactiveId = 0;
        } else if (_input->mouseIsDown) {
            _activeId = id;
        }
    }
    if (IsActive(id)) {
        assert(IsPreactive(id));
        if (_input->mouseIsUp) {
            _activeId = 0;
        }
    }

    float stroke = 2.5f;
    if (IsActive(id) || IsPreactive(id)) {
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
    } else if (IsActive(id)) {
        float pxToLevelScalar = 1.f / 150.f;
        float newLevel = levelToUse - pxToLevelScalar * _input->mouseYDelta;
        newLevel = utility::Clamp(newLevel, -zero, 1.f - zero);
        levelToUse = newLevel;
        *level = levelToUse;
    }

    // Normalize level from 0 to 1
    float normalizedLevel = levelToUse + zero;
    if (normalizedLevel < -0.001f || normalizedLevel > 1.001f) {
        SDL_Log("normalizedLevel = %f", normalizedLevel);
        assert(false);
    }

    float levelDeg = startDeg + (endDeg - startDeg) * normalizedLevel;
    float zeroDeg = startDeg + (endDeg - startDeg) * zero;

    // Draw knob background and arcs
    DrawFilledCircle(cx, cy, r, KNOB_BG);
    if (normalizedLevel < zero) {
        // startDeg -> (level -> zero) -> endDeg
        DrawArc(cx, cy, r - 1.5f - stroke, startDeg, levelDeg, stroke, KNOB_INACTIVE_GREY);
        DrawArc(cx, cy, r - 1.5f - stroke, levelDeg, zeroDeg, stroke, KNOB_ACTIVE_PURPLE);
        DrawArc(cx, cy, r - 1.5f - stroke, zeroDeg, endDeg, stroke, KNOB_INACTIVE_GREY);
    } else {
        // startDeg -> (zero -> level) -> endDeg
        DrawArc(cx, cy, r - 1.5f - stroke, startDeg, zeroDeg, stroke, KNOB_INACTIVE_GREY);
        DrawArc(cx, cy, r - 1.5f - stroke, zeroDeg, levelDeg, stroke, KNOB_ACTIVE_PURPLE);
        DrawArc(cx, cy, r - 1.5f - stroke, levelDeg, endDeg, stroke, KNOB_INACTIVE_GREY);
    }

    // Draw line for knob level indicator
    float outerRadius = r - 1.5f - stroke/2.f;
    nvgSave(_nvg);
    nvgTranslate(_nvg, cx, cy);
    nvgRotate(_nvg, DegToRad(levelDeg));
    DrawLine(.3f * outerRadius, 0, outerRadius, 0, stroke, WHITE);
    nvgRestore(_nvg);

    // Knob label at the bottom
    RoundRectLabel(text, x, y + KNOB_WIDTH + KNOB_LABEL_GAP, 12, KNOB_LABEL_BG, WHITE, KNOB_WIDTH, LABEL_HEIGHT);

    // Overlay text of current value. Only visible if preactive or active.
    float overlayOpacity = 0.0f;
    if (IsActive(id)) {
        overlayOpacity = 0.6f;
    } else if (IsPreactive(id)) {
        overlayOpacity = 0.3f;
    }
    NVGcolor bg = KNOB_LABEL_BG;
    bg.a = overlayOpacity;
    NVGcolor fg = WHITE;
    fg.a = overlayOpacity;
    RoundRectLabel(valuetext, cx-r, cy-1.6f*r, 12, bg, fg, std::nullopt, LABEL_HEIGHT * 0.8f);
}

bool UI::ArrowButton(float x, float y, float radius, NVGcolor bgColor, NVGcolor fgColor, bool isLeft) {
    // TODO - handle active logic

    // Button
    UI::DrawFilledCircle(x, y, radius, bgColor);

    // Arrow
    {
        nvgSave(_nvg);

        float xTranslate = (isLeft ? x - radius/2.f : x + radius/2.f);
        nvgTranslate(_nvg, xTranslate, y);

        float rotate1 = (isLeft ? 240.f : -240.f);
        nvgRotate(_nvg, DegToRad(rotate1));
        DrawLine(0.f, 0.f, 0.f, radius, 1.f, ALMOST_WHITE);

        float rotate2 = (isLeft ? 60.f : -60.f);
        nvgRotate(_nvg, DegToRad(rotate2));
        DrawLine(0.f, 0.f, 0.f, radius, 1.f, ALMOST_WHITE);

        nvgRestore(_nvg);
    }

    return false;
}

void UI::Oscillator(const char* name, float x, float y) {
    size_t id = ScopedId(_idStack, name).value();

    float num_knobs = 4.f;
    float rw = PAD + (WAVEFORM_WIDTH + PAD) + num_knobs * (KNOB_WIDTH + PAD);
    float rh = 2.f * PAD + WAVEFORM_HEIGHT;

    RoundRectLabel(name, x - 12, y - 24, 14, TRANSPARENT, WHITE);

    // Oscillator background
    nvgBeginPath(_nvg);
    nvgRoundedRect(_nvg, x, y, rw, rh, 5.f);
    nvgFillColor(_nvg, OSC_ENABLED_GREY);
    nvgStrokeWidth(_nvg, 2.f);
    nvgStrokeColor(_nvg, DARK_GREY);
    nvgFill(_nvg);
    nvgStroke(_nvg);

    float xoff = x + PAD;
    float yoff = y + PAD;

    //-----------------------
    // Oscillator selector
    //-----------------------
    bool waveformLeft = false;
    bool waveformRight = false;
    {
        // Background
        nvgBeginPath(_nvg);
        nvgRoundedRect(_nvg, xoff, yoff, WAVEFORM_WIDTH, WAVEFORM_HEIGHT, 5.f);
        nvgFillColor(_nvg, DARK_GREY);
        nvgFill(_nvg);
        nvgStroke(_nvg);

        // Left/right selection buttons
        float buttonRadius = 10.f;
        float buttonOffset = PAD/3.f + buttonRadius;
        float leftButtonCenterX = xoff + buttonOffset;
        float rightButtonCenterX = xoff + WAVEFORM_WIDTH - PAD/3.f - buttonRadius;
        float buttonCenterY = yoff + buttonOffset;
        if (ArrowButton(leftButtonCenterX, buttonCenterY, buttonRadius, OSC_ENABLED_GREY, ALMOST_WHITE, true)) {
            // TODO - handle left
        }
        if (ArrowButton(rightButtonCenterX, buttonCenterY, buttonRadius, OSC_ENABLED_GREY, ALMOST_WHITE, false)) {
            // TODO - handle right
        }

        // Oscillator name
        Label("Sine", xoff + WAVEFORM_WIDTH/2.f, buttonCenterY, 14, TRANSPARENT, ALMOST_WHITE);

        // TODO - visualize oscillator waveform
    }

    xoff += (WAVEFORM_WIDTH + PAD);

    //-----------------------
    // Knobs
    //-----------------------
    float levelValue = _synth->osc.volume;
    char levelText[16] = {};
    snprintf(levelText, sizeof(levelText), "%3.1f%%", fabs(levelValue * 100.f));
    Knob("LEVEL", xoff, yoff, 0.f, 0.7f, &levelValue, levelText);
    _synth->osc.volume = levelValue;

    xoff += (KNOB_WIDTH + PAD);

    float panValue = _synth->osc.pan;
    int left = (int)(round(100.f * utility::Map(panValue, -.5f, .5f, 1.0f, 0.0f)));
    int right = 100 - left;
    char panText[16] = {};
    snprintf(panText, sizeof(panText), "%dL/%dR", left, right);
    Knob("PAN", xoff, yoff, 0.5f, 0.0f, &panValue, panText);
    _synth->osc.pan = panValue;

    xoff += (KNOB_WIDTH + PAD);

    float coarseValue = _synth->osc.coarsePitch;
    float coarseKnobLevel = utility::Map(coarseValue, -36.f, 36.f, -.5, .5);
    char coarseText[16] = {};
    snprintf(coarseText, sizeof(coarseText), "%d st", (int32_t)round(coarseValue));
    Knob("PITCH", xoff, yoff, 0.5f, 0.0f, &coarseKnobLevel, coarseText);
    coarseValue = utility::Map(coarseKnobLevel, -.5f, .5f, -36.f, 36.f);
    _synth->osc.coarsePitch = coarseValue;

    xoff += (KNOB_WIDTH + PAD);

    float fineValue = _synth->osc.finePitch;
    float fineKnobLevel = utility::Map(fineValue, -100.f, 100.f, -.5f, .5f);
    char fineText[16] = {};
    snprintf(fineText, sizeof(fineText), "%3.1f cents", fineValue);
    Knob("FINE", xoff, yoff, 0.5f, 0.0f, &fineKnobLevel, fineText);
    fineValue = utility::Map(fineKnobLevel, -.5f, .5f, -100.f, 100.f);
    _synth->osc.finePitch = fineValue;
}

void UI::Draw() {
    ClearBackground(BG_GREY);

    _synth->osc.noteActive = _input->mouseIsDown;

    nvgBeginFrame(_nvg, WINDOW_WIDTH, WINDOW_HEIGHT, 1.f);

    Oscillator("OSC A", 100.f, 100.f);

    // TODO - Cleanup usage of RoundedRectLabel, use just Label

    nvgEndFrame(_nvg);
}
