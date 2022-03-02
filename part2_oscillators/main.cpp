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
// Signed 16-bit samples, in system byte order
#define SAMPLE_FORMAT AUDIO_S16SYS
// Smaller buffer == lower latency, but too low can cause crackling.
#define SAMPLES_PER_BUFFER 256 // (256 / 48000) = 5.333 ms latency
// Max volume, scaling factor from 0.0 to 1.0
#define VOLUME 0.025 // about -32 dB

static SDL_Renderer* _renderer;
static SDL_Window* _window;
static SDL_AudioDeviceID _audioDevice;
static bool _playSound;

static void close(void) {
    SDL_Log("Closing");
    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
    }
    if (_window) {
        SDL_DestroyWindow(_window);
    }
    SDL_CloseAudioDevice(_audioDevice);
    SDL_Quit();
}

static void audioCallback(void* userdata, Uint8* stream, int len) {
    constexpr double twoPi = 2.0 * M_PI;
    constexpr double secondsPerSample = 1.0 / SAMPLE_RATE_HZ;

    // Sine wave, A440 Hz
    constexpr double freqHz = 440;
    constexpr double periodS = 1.0 / freqHz;
    // For s16, range is [-32768, 32767]
    constexpr double maxAmp = 32767.0 * VOLUME;

    static double t = 0.0;
    while (len > 0) {
        double y = 0.0;
        if (_playSound) {
#if 0
            // Equation for a sine wave:
            //    y(t) = A * sin(2 * PI * f * t + shift)
            y = maxAmp * sin(twoPi * freqHz * t);
#endif
            double dutyCycle = 0.5;
            if (t < (periodS * dutyCycle)) {
                y = maxAmp;
            } else {
                y = -maxAmp;
            }
        }

        int16_t* left = (int16_t*)(stream);
        int16_t* right = (int16_t*)(stream + 2);
        *left = (int16_t)y;
        *right = (int16_t)y;

        t += secondsPerSample;
        if (t > periodS) {
             t -= periodS;
        }
        stream += 4;
        len -= 4;
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

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

    // Create a window
    _window = SDL_CreateWindow(
        "Synth",
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

    bool loopShouldStop = false;
    SDL_Event event;

    // Main SDL Loop
    SDL_Log("Starting main loop");
    while (!loopShouldStop) {
        // Check for events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                loopShouldStop = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                _playSound = true;
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                _playSound = false;
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
