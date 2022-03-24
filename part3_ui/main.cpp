#include "synth.h"
#include <glad/glad.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#if IS_WASM_BUILD
#include <emscripten.h>
#endif

#define RETURN_1_IF_FALSE(expr) if ((!expr)) { return 1; }

static std::unique_ptr<Synth> _synth;

// TODO - Move this somewhere else
static void audioCallback(void* userdata, Uint8* stream, int len) {
    const double periodS = 1.0 / _synth->freqHz;

    static double t = 0.0;
    while (len > 0) {
        double y = 0.0;
        if (_synth->soundEnabled) {
            y = VOLUME * _synth->osc.getSample(t, _synth->freqHz);
        }

        // Populate left and right channels with the same sample
        float* left = (float*)(stream);
        float* right = (float*)(stream + 4);
        *left = (float)y;
        *right = (float)y;

        t += dt;
        if (t >= periodS) { // wraparound
            if (_synth->start) {
                _synth->start = false;
                _synth->soundEnabled = true;
            } else if (_synth->stop) {
                _synth->stop = false;
                _synth->soundEnabled = false;
            }
            t -= periodS;
        }

        // Advance forward in the stream
        stream += (2 * sizeof(float));
        len -= (2 * sizeof(float));
    }
}

// TODO - Move this somewhere else
static void checkInputEvents(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            _synth->shouldQuit = true;
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
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
            constexpr double dFreqMaxHz = DEFAULT_FREQ;
            if (_synth->soundEnabled) {
                _synth->freqHz -= dFreqMaxHz * ((double)event.motion.yrel / (double)WINDOW_HEIGHT);
                // Clip if freq goes too low or high
                _synth->freqHz = std::max(_synth->freqHz, 1.0);
                _synth->freqHz = std::min(_synth->freqHz, SAMPLE_RATE_HZ / 2.0);
            }
        }
    }
}

static void loopOnce(void* arg) {
    checkInputEvents();
    // TODO - update
    _synth->ui.draw();
    SDL_GL_SwapWindow(_synth->sdl._window);
}

int main(int argc, char* argv[]) {
    _synth = std::make_unique<Synth>();

    RETURN_1_IF_FALSE(_synth->sdl.init(
            "Synth (part 3)",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SAMPLE_RATE_HZ,
            SAMPLES_PER_BUFFER,
            audioCallback));
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
