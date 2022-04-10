#include "input.h"
#include "synth.h"

void Input::PollEvents() {
    SDL_Event event;

    // Auto-clear per-frame data
    mouseWentUp = false;
    mouseWentDown = false;
    mouseDoubleClick = false;
    mouseYDelta = 0.f;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            _synth->running = false;
        } else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;
            if (key == SDLK_ESCAPE) {
                SDL_Log("Escape key");
                _synth->running = false;
            }
            _keyIsPressed[key] = true;
        } else if (event.type == SDL_KEYUP) {
            SDL_Keycode key = event.key.keysym.sym;
            _keyIsPressed[key] = false;
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            mouseWentUp = true;
            mouseIsDown = false;
            mouseIsUp = true;
            uint32_t nowMs = SDL_GetTicks();
            if (nowMs - _lastMouseUpMs < 250) {
                mouseDoubleClick = true;
            }
            _lastMouseUpMs = SDL_GetTicks();
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            mouseWentDown = true;
            mouseIsDown = true;
            mouseIsUp = false;
        } else if (event.type == SDL_MOUSEMOTION) {
            mouseYDelta = event.motion.y - mouseY;
            mouseX = event.motion.x;
            mouseY = event.motion.y;
        }
    }
}

bool Input::IsKeyPressed(SDL_Keycode key) const {
    auto search = _keyIsPressed.find(key);
    if (search != _keyIsPressed.end()) {
        return search->second;
    }
    return false;
}
