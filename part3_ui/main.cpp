#include "constants.h"
#include "ui.h"
#include "oscillator.h"
#include <SDL_ttf.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#if IS_WASM_BUILD
#include <emscripten.h>
#endif

struct Synth {
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_AudioDeviceID audioDevice;
    bool loopShouldStop = false;
    bool start = false;
    bool stop = false;
    bool soundEnabled = false;
    double freqHz = DEFAULT_FREQ;
    Oscillator::Context oscContext;
};

static void close(const Synth& synth) {
    SDL_Log("Closing");
    SDL_CloseAudioDevice(synth.audioDevice);
    if (synth.renderer) {
        SDL_DestroyRenderer(synth.renderer);
    }
    if (synth.window) {
        SDL_DestroyWindow(synth.window);
    }
    SDL_Quit();
}

static void audioCallback(void* userdata, Uint8* stream, int len) {
    assert(len == SAMPLES_PER_BUFFER * NUM_SOUND_CHANNELS * sizeof(float));
    Synth& synth = *(Synth*)userdata;
    const double periodS = 1.0 / synth.freqHz;

    static double t = 0.0;
    while (len > 0) {
        double y = 0.0;
        if (synth.soundEnabled) {
            y = VOLUME * Oscillator::getSample(synth.oscContext, t, synth.freqHz);
        }

        // Populate left and right channels with the same sample
        float* left = (float*)(stream);
        float* right = (float*)(stream + 4);
        *left = (float)y;
        *right = (float)y;

        t += dt;
        if (t >= periodS) { // wraparound
            if (synth.start) {
                synth.start = false;
                synth.soundEnabled = true;
            } else if (synth.stop) {
                 synth.stop = false;
                synth.soundEnabled = false;
            }
            t -= periodS;
        }

        // Advance forward in the stream
        stream += (2 * sizeof(float));
        len -= (2 * sizeof(float));
    }
}

static void loop(void* arg) {
    Synth& synth = *(Synth*)arg;
    // Check for events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            synth.loopShouldStop = true;
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            synth.loopShouldStop = true;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                synth.start = true;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                synth.stop = true;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                Oscillator::nextFn(synth.oscContext);
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            constexpr double dFreqMaxHz = DEFAULT_FREQ;
            if (synth.soundEnabled) {
                synth.freqHz -= dFreqMaxHz * ((double)event.motion.yrel / (double)WINDOW_HEIGHT);
                // Clip if freq goes too low or high
                synth.freqHz = std::max(synth.freqHz, 1.0);
                synth.freqHz = std::min(synth.freqHz, SAMPLE_RATE_HZ / 2.0);
            }
        }
    }

    // Render frame with dark gray background
    SDL_SetRenderDrawColor(synth.renderer, 25, 25, 25, 255);
    SDL_RenderClear(synth.renderer);
    UI::drawWaveform(synth.renderer, synth.oscContext.fn, synth.freqHz, synth.soundEnabled);
    SDL_RenderPresent(synth.renderer);
}

int main(int argc, char* argv[]) {
    Synth synth = {};

    // Initialize SDL
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    // Create a window
    synth.window = SDL_CreateWindow(
        "Synth (Part 2)",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        0
    );
    if (synth.window == nullptr) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        close(synth);
        return -3;
    }

    // Create a renderer
    synth.renderer = SDL_CreateRenderer(
            synth.window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (synth.renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        close(synth);
        return -4;
    }
    SDL_RenderSetLogicalSize(synth.renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Initialize Audio
    SDL_AudioSpec desired = {};
    desired.freq = SAMPLE_RATE_HZ;
    desired.format = SAMPLE_FORMAT;
    desired.channels = NUM_SOUND_CHANNELS;
    desired.samples = SAMPLES_PER_BUFFER;
    desired.callback = audioCallback;
    desired.userdata = &synth;

    SDL_AudioSpec actual = {};
    synth.audioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, &actual, 0);
    if (synth.audioDevice <= 0) {
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

    SDL_PauseAudioDevice(synth.audioDevice, 0);

#ifdef IS_WASM_BUILD
    const int simulate_infinite_loop = 1;
    const int fps = 60;
    emscripten_set_main_loop_arg(loop, &synth, fps, simulate_infinite_loop);
#else
    // Main SDL Loop
    SDL_Log("Starting main loop");
    while (!synth.loopShouldStop) {
        loop(&synth);
    }
#endif

    close(synth);
    return 0;
}
