#include "constants.h"
#include "ui.h"
#include "synth.h"
#include "oscillator.h"
#include <SDL_ttf.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#if IS_WASM_BUILD
#include <emscripten.h>
#endif

static Synth _synth;

Synth::Synth()
    : osc(new Oscillator())
    , ui(new UI()) {
}

static void close() {
    SDL_Log("Closing");
    SDL_CloseAudioDevice(_synth.audioDevice);
    if (_synth.renderer) {
        SDL_DestroyRenderer(_synth.renderer);
    }
    if (_synth.window) {
        SDL_DestroyWindow(_synth.window);
    }
    TTF_Quit();
    SDL_Quit();
}

static void audioCallback(void* userdata, Uint8* stream, int len) {
    assert(len == SAMPLES_PER_BUFFER * NUM_SOUND_CHANNELS * sizeof(float));
    const double periodS = 1.0 / _synth.freqHz;

    static double t = 0.0;
    while (len > 0) {
        double y = 0.0;
        if (_synth.soundEnabled) {
            y = VOLUME * _synth.osc->getSample(t, _synth.freqHz);
        }

        // Populate left and right channels with the same sample
        float* left = (float*)(stream);
        float* right = (float*)(stream + 4);
        *left = (float)y;
        *right = (float)y;

        t += dt;
        if (t >= periodS) { // wraparound
            if (_synth.start) {
                _synth.start = false;
                _synth.soundEnabled = true;
            } else if (_synth.stop) {
                _synth.stop = false;
                _synth.soundEnabled = false;
            }
            t -= periodS;
        }

        // Advance forward in the stream
        stream += (2 * sizeof(float));
        len -= (2 * sizeof(float));
    }
}

static void loop(void* arg) {
    // Check for events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            _synth.loopShouldStop = true;
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            _synth.loopShouldStop = true;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                _synth.start = true;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                _synth.stop = true;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                _synth.osc->nextFn();
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            constexpr double dFreqMaxHz = DEFAULT_FREQ;
            if (_synth.soundEnabled) {
                _synth.freqHz -= dFreqMaxHz * ((double)event.motion.yrel / (double)WINDOW_HEIGHT);
                // Clip if freq goes too low or high
                _synth.freqHz = std::max(_synth.freqHz, 1.0);
                _synth.freqHz = std::min(_synth.freqHz, SAMPLE_RATE_HZ / 2.0);
            }
        }
    }

    _synth.ui->draw();
    SDL_RenderPresent(_synth.renderer);
}

int main(int argc, char* argv[]) {
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    if (0 != TTF_Init()) {
        SDL_Log("TTF_Init failed, error: %s", TTF_GetError());
        return -1;
    }

    _synth.window = SDL_CreateWindow(
        "Synth (Part 3)",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        0
    );
    if (_synth.window == nullptr) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        close();
        return -3;
    }

    // Create a renderer
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    _synth.renderer = SDL_CreateRenderer(
            _synth.window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (_synth.renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        close();
        return -4;
    }
    SDL_RenderSetLogicalSize(_synth.renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Initialize Audio
    SDL_AudioSpec desired = {};
    desired.freq = SAMPLE_RATE_HZ;
    desired.format = SAMPLE_FORMAT;
    desired.channels = NUM_SOUND_CHANNELS;
    desired.samples = SAMPLES_PER_BUFFER;
    desired.callback = audioCallback;

    SDL_AudioSpec actual = {};
    _synth.audioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, &actual, 0);
    if (_synth.audioDevice <= 0) {
        SDL_Log("Could not open audio device: %s", SDL_GetError());
        return -2;
    }
    if ((desired.format != actual.format) || (desired.samples != actual.samples)) {
        SDL_Log("Could not get desired audio format");
        return -2;
    }

    SDL_Log("-------------------");
    SDL_Log("sample rate: %d", actual.freq);
    SDL_Log("channels:    %d", actual.channels);
    SDL_Log("samples:     %d", actual.samples);
    SDL_Log("size:        %d", actual.size);
    SDL_Log("------------------");

    SDL_PauseAudioDevice(_synth.audioDevice, 0);

    _synth.ui->init(&_synth);

#ifdef IS_WASM_BUILD
    const int simulate_infinite_loop = 1;
    const int fps = 60;
    emscripten_set_main_loop_arg(loop, NULL, fps, simulate_infinite_loop);
#else
    // Main SDL Loop
    SDL_Log("Starting main loop");
    while (!_synth.loopShouldStop) {
        loop(&_synth);
    }
#endif

    close();
    return 0;
}
