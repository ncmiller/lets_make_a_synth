#include <SDL.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

// Window Configuration
#define WINDOW_WIDTH 360
#define WINDOW_HEIGHT 640

// Audio Configuration
#define SAMPLE_RATE_HZ 48000
#define NUM_SOUND_CHANNELS 2
// 32-bit float samples, in system byte order
#define SAMPLE_FORMAT AUDIO_F32SYS
// Smaller buffer == lower latency, but too low can cause crackling.
#define SAMPLES_PER_BUFFER 64 // (64 / 48000) = 1.333 ms latency
// Max volume, scaling factor from 0.0 to 1.0
#define VOLUME 0.025 // about -32 dB


static SDL_Renderer* _renderer;
static SDL_Window* _window;
static SDL_AudioDeviceID _audioDevice;
static bool _loopShouldStop = false;
static bool _start = false;
static bool _stop = false;
static bool _soundEnabled = false;

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

static void audioCallback(void* userdata, Uint8* stream, int len) {
    assert(len == SAMPLES_PER_BUFFER * NUM_SOUND_CHANNELS * sizeof(float));
    constexpr double twoPi = 2.0 * M_PI;
    constexpr double freqHz = 440.0;
    constexpr double periodS = 1.0 / freqHz;
    constexpr double dt = 1.0 / SAMPLE_RATE_HZ;

    static double t = 0.0;
    while (len > 0) {
        double y = 0.0;
        if (_soundEnabled) {
            y = VOLUME * sin(twoPi * freqHz * t);
        }

        // Populate left and right channels with the same sample
        float* left = (float*)(stream);
        float* right = (float*)(stream + 4);
        *left = (float)y;
        *right = (float)y;

        t += dt;
        if (t >= periodS) { // wraparound
            assert(y <= 0.001);
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

    // Main SDL Loop
    SDL_Event event;
    SDL_Log("Starting main loop");
    while (!_loopShouldStop) {
        // Check for events
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

    close();
    return 0;
}
