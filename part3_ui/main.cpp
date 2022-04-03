#include "synth.h"
#include "audio.h"
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#if IS_WASM_BUILD
#include <emscripten.h>
#endif

#define RETURN_1_IF_FALSE(expr) if ((!expr)) { return 1; }

static std::unique_ptr<Synth> _synth;

// TODO - Move this somewhere else
static void checkInputEvents(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            _synth->shouldQuit = true;
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            // TODO - sometimes we get here even though I didn't hit escape
            SDL_Log("Escape key");
            _synth->shouldQuit = true;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                _synth->start = true;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                _synth->stop = true;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                _synth->osc.nextFn();
            }
        } else if (event.type == SDL_MOUSEMOTION) {
        }

        // Also notify UI
        _synth->ui.onControlEvent(event);
    }
}

static void loopOnce(void* arg) {
    checkInputEvents();
    _synth->ui.draw();
    SDL_GL_SwapWindow(_synth->sdl._window);
}

int main(int argc, char* argv[]) {
    _synth = std::make_unique<Synth>();

    RETURN_1_IF_FALSE(_synth->sdl.init(
            "Synth (part 3)",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            (uint32_t)SAMPLE_RATE_HZ,
            SAMPLES_PER_BUFFER,
            audio::AudioCallback,
            (void*)_synth.get()));
    RETURN_1_IF_FALSE(_synth->osc.init(_synth.get()));
    RETURN_1_IF_FALSE(_synth->ui.init(_synth.get()));

#ifdef IS_WASM_BUILD
    emscripten_set_main_loop_arg(loopOnce, NULL, 60, 1);
#else
    SDL_Log("Starting main loop");
    while (!_synth->shouldQuit) {
        loopOnce(NULL);
    }
#endif

    SDL_Log("Exiting");
    return 0;
}
