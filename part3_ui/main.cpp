#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <algorithm>
#if IS_WASM_BUILD
#include <emscripten.h>
#endif

// Window Configuration
#define WINDOW_WIDTH 360
#define WINDOW_HEIGHT 360

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
#define DEFAULT_FREQ 220.0

constexpr double _dt = 1.0 / SAMPLE_RATE_HZ;

typedef double (*OscillatorFn)(double t, double freqHz);
static double sine(double t, double freqHz);
static double square(double t, double freqHz);
static double triangle(double t, double freqHz);
static double saw(double t, double freqHz);
static double whitenoise(double t, double freqHz);

static SDL_Renderer* _renderer;
static SDL_Window* _window;
static SDL_AudioDeviceID _audioDevice;
static bool _loopShouldStop = false;
static bool _start = false;
static bool _stop = false;
static bool _soundEnabled = false;
static double _freqHz = DEFAULT_FREQ;
static OscillatorFn _oscFn = sine;

static void nextOsc() {
    if (_oscFn == sine) {
        _oscFn = square;
    } else if (_oscFn == square) {
        _oscFn = triangle;
    } else if (_oscFn == triangle) {
        _oscFn = saw;
    } else if (_oscFn == saw) {
        _oscFn = whitenoise;
    } else if (_oscFn == whitenoise) {
        _oscFn = sine;
    }
}

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
    t = fmod(t, 1.0 / freqHz);
    constexpr double twoPi = 2.0 * M_PI;
    return sin(twoPi * freqHz * t);
}

static double square(double t, double freqHz) {
    t = fmod(t, 1.0 / freqHz);
    const double halfPeriodS = 1.0 / freqHz / 2.0;
    if (t < halfPeriodS) {
        return 1;
    } else {
        return -1;
    }
}

static double saw(double t, double freqHz) {
    t = fmod(t, 1.0 / freqHz);
    const double periodS = 1.0 / freqHz;
    const double percentComplete = t / periodS;
    // scale and offset to get it in range [-1, 1]
    return 2.0 * percentComplete - 1.0;
}

static double triangle(double t, double freqHz) {
    t = fmod(t, 1.0 / freqHz);
    const double periodS = 1.0 / freqHz;
    const double percentComplete = t / periodS;
    if (percentComplete <= 0.5) {
        // 1 down to -1
        return 1.0 - 4.0 * percentComplete;
    } else {
        // -1 up to 1
        return -1.0 + 4.0 * (percentComplete - 0.5);
    }
}

static double whitenoise(double t, double freqHz) {
    double rand_normalized = (double)rand() / (double)RAND_MAX;
    return 2.0 * rand_normalized - 1.0;
}

static void audioCallback(void* userdata, Uint8* stream, int len) {
    assert(len == SAMPLES_PER_BUFFER * NUM_SOUND_CHANNELS * sizeof(float));
    const double periodS = 1.0 / _freqHz;

    static double t = 0.0;
    while (len > 0) {
        double y = 0.0;
        if (_soundEnabled) {
            y = VOLUME * _oscFn(t, _freqHz);
        }

        // Populate left and right channels with the same sample
        float* left = (float*)(stream);
        float* right = (float*)(stream + 4);
        *left = (float)y;
        *right = (float)y;

        t += _dt;
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

static void drawCircle(int centerX, int centerY, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(_renderer, centerX + x, centerY + y);
            }
        }
    }
}

static void drawWaveform(void) {
    SDL_SetRenderDrawColor(_renderer, 50, 205, 50, 255); // lime green

    int padding = (int)(0.1 * (double)WINDOW_WIDTH);
    int drawingWidth = WINDOW_WIDTH - 2 * padding;

    // Draw the waveform from left to right, as if starting
    // from the waveform point at t = 0.
    const double periodS = 1.0 / DEFAULT_FREQ;
    for (double t = 0.0; t < periodS; t += _dt) {
        double y = _oscFn(t, _freqHz);
        // Convert (t,y) to 2-D coord (px,py)
        int px = padding + (int)((t / periodS) * drawingWidth);
        int py = padding + (int)((drawingWidth / 2) * (1.0 - y));
        int radius = (_soundEnabled ? 4 : 1);
        drawCircle(px, py, radius);
    }
}

static void loop(void* arg) {
    // Check for events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            _loopShouldStop = true;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                _start = true;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                _stop = true;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                nextOsc();
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            constexpr double dFreqMaxHz = DEFAULT_FREQ;
            if (_soundEnabled) {
                _freqHz -= dFreqMaxHz * ((double)event.motion.yrel / (double)WINDOW_HEIGHT);
                // Clip if freq goes too low or high
                _freqHz = std::max(_freqHz, 1.0);
                _freqHz = std::min(_freqHz, SAMPLE_RATE_HZ / 2.0);
            }
        }
    }

    // Render frame with dark gray background
    SDL_SetRenderDrawColor(_renderer, 25, 25, 25, 255);
    SDL_RenderClear(_renderer);
    drawWaveform();
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
        "Synth (Part 2)",
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
