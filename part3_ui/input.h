#pragma once

#include <SDL.h>
#include <unordered_map>

struct Synth;

class Input {
public:
    bool Init(Synth* synth) {
        _synth = synth;
        return true;
    }
    void PollEvents();

    // e.g. IsKeyPressed(SDLK_a)
    bool IsKeyPressed(SDL_Keycode key) const;

    bool mouseIsDown = false;
    bool mouseIsUp = true;
    float mouseX = 0.f;
    float mouseY = 0.f;
    float mouseYDelta = 0.f;

    // Auto-cleared on every frame
    bool mouseWentUp = false;
    bool mouseWentDown = false;
    bool mouseDoubleClick = false;

private:
    Synth* _synth; // parent
    float _lastMouseUpMs = 0.f;
    std::unordered_map<SDL_Keycode, bool> _keyIsPressed;
};
