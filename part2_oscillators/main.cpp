#include <SDL.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#if IS_WASM_BUILD
#include <emscripten.h>
#endif

// Window Configuration
#define WINDOW_WIDTH 360
#define WINDOW_HEIGHT 640

// Audio Configuration
#define SAMPLE_RATE_HZ 48000
#define NUM_SOUND_CHANNELS 2
#define SAMPLE_FORMAT AUDIO_F32SYS // 32-bit float
#define VOLUME 0.025 // max volume, about -32 dB
#ifdef IS_WASM_BUILD
#define SAMPLES_PER_BUFFER 256 // (256 / 48000) = 5.333 ms latency
#else
#define SAMPLES_PER_BUFFER 64 // (64 / 48000) = 1.333 ms latency
#endif


static SDL_Renderer* _renderer;
static SDL_Window* _window;
static SDL_AudioDeviceID _audioDevice;
static bool _loopShouldStop = false;
static bool _start = false;
static bool _stop = false;
static bool _soundEnabled = false;

typedef double (*OscillatorFn)(double t, double freqHz);

static void close(void) {
    SDL_Log("Closing");
    SDL_CloseAudioDevice(_audioDevice);
    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
    }
    if (_window) {
        SDL_DestroyWindow(_window);
    }
    SDL_Quit();
}

static double sine(double t, double freqHz) {
    constexpr double twoPi = 2.0 * M_PI;
    return sin(twoPi * freqHz * t);
}

static double square(double t, double freqHz) {
    const double halfPeriodS = 1.0 / freqHz / 2.0;
    if (t < halfPeriodS) {
        return 1;
    } else {
        return -1;
    }
}

static double saw(double t, double freqHz) {
    const double periodS = 1.0 / freqHz;
    const double percentComplete = t / periodS;
    // scale and offset to get it in range [-1, 1]
    return 2.0 * percentComplete - 1.0;
}

static double whitenoise(double t, double freqHz) {
    double rand_normalized = rand() / RAND_MAX;
    return 2.0 * rand_normalized - 1.0;
}

static void audioCallback(void* userdata, Uint8* stream, int len) {
    assert(len == SAMPLES_PER_BUFFER * NUM_SOUND_CHANNELS * sizeof(float));
    constexpr double freqHz = 110.0;
    constexpr double periodS = 1.0 / freqHz;
    constexpr double dt = 1.0 / SAMPLE_RATE_HZ;
    OscillatorFn oscFn = whitenoise;

    static double t = 0.0;
    while (len > 0) {
        double y = 0.0;
        if (_soundEnabled) {
            y = VOLUME * oscFn(t, freqHz);
        }

        // Populate left and right channels with the same sample
        float* left = (float*)(stream);
        float* right = (float*)(stream + 4);
        *left = (float)y;
        *right = (float)y;

        t += dt;
        if (t >= periodS) { // wraparound
            if (_start) {
                _start = false;
                _soundEnabled = true;
            } else if (_stop) {
                 _stop = false;
                _soundEnabled = false;
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
            _loopShouldStop = true;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            _start = true;
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            _stop = true;
        }
    }

    // Render frame with dark gray background
    SDL_RenderClear(_renderer);
    SDL_SetRenderDrawColor(_renderer, 25, 25, 25, 255);
    SDL_RenderPresent(_renderer);
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }


    // Create a window
    _window = SDL_CreateWindow(
        "Synth (Part 1)",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        0
    );
    if (_window == nullptr) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        close();
        return -3;
    }

    // Create a renderer
    _renderer = SDL_CreateRenderer(
            _window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (_renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        close();
        return -4;
    }
    SDL_RenderSetLogicalSize(_renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Initialize Audio
    SDL_AudioSpec desired = {};
    desired.freq = SAMPLE_RATE_HZ;
    desired.format = SAMPLE_FORMAT;
    desired.channels = NUM_SOUND_CHANNELS;
    desired.samples = SAMPLES_PER_BUFFER;
    desired.callback = audioCallback;

    SDL_AudioSpec actual = {};
    _audioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, &actual, 0);
    if (_audioDevice <= 0) {
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

    SDL_PauseAudioDevice(_audioDevice, 0);

#ifdef IS_WASM_BUILD
    const int simulate_infinite_loop = 1;
    const int fps = 60;
    emscripten_set_main_loop_arg(loop, NULL, fps, simulate_infinite_loop);
#else
    // Main SDL Loop
    SDL_Log("Starting main loop");
    while (!_loopShouldStop) {
        loop(NULL);
    }
#endif

    close();
    return 0;
}
