#include "input.h"
#include "synth.h"
#include <SDL.h>

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
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            // TODO - sometimes we get here even though escape was not pressed
            SDL_Log("Escape key");
            _synth->running = false;
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
