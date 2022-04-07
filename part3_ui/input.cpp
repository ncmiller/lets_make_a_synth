#include "input.h"
#include "synth.h"
#include <SDL.h>

void Input::PollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            _synth->running = false;
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            // TODO - sometimes we get here even though I didn't hit escape
            SDL_Log("Escape key");
            _synth->running = false;
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_RIGHT) {
                _synth->osc.NextFn();
            }
        }

        // Also notify UI
        _synth->ui.OnControlEvent(event);
    }
}
