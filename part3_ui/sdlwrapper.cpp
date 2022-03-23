#include "sdlwrapper.h"

#define SAMPLE_FORMAT AUDIO_F32SYS // 32-bit float
#define NUM_SOUND_CHANNELS 2

#define RETURN_FALSE_IF_FALSE(expr) if ((!expr)) { return false; }

bool SDLWrapper::init(
        const char* winTitle,
        uint32_t widthPx,
        uint32_t heightPx,
        uint32_t audioSampleRateHz,
        uint16_t audioSamplesPerBuffer,
        SDL_AudioCallback audioCallback) {
    RETURN_FALSE_IF_FALSE(initWindow(winTitle, widthPx, heightPx));
    RETURN_FALSE_IF_FALSE(initRenderer(widthPx, heightPx));
    RETURN_FALSE_IF_FALSE(initAudio(audioSampleRateHz, audioSamplesPerBuffer, audioCallback));
    return true;
}

bool SDLWrapper::initWindow(const char* title, uint32_t widthPx, uint32_t heightPx) {
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    _window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        (int)widthPx, (int)heightPx,
        0
    );
    if (_window == nullptr) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool SDLWrapper::initRenderer(uint32_t widthPx, uint32_t heightPx) {
    _renderer = SDL_CreateRenderer(
            _window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (_renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return false;
    }
    SDL_RenderSetLogicalSize(_renderer, (int)widthPx, (int)heightPx);
    return true;
}

bool SDLWrapper::initAudio(uint32_t sampleRateHz, uint16_t samplesPerBuffer, SDL_AudioCallback audioCallback) {
    // Initialize Audio
    SDL_AudioSpec desired = {};
    desired.freq = (int)sampleRateHz;
    desired.format = SAMPLE_FORMAT;
    desired.channels = NUM_SOUND_CHANNELS;
    desired.samples = samplesPerBuffer;
    desired.callback = audioCallback;

    SDL_AudioSpec actual = {};
    _audioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, &actual, 0);
    if (_audioDevice <= 0) {
        SDL_Log("Could not open audio device: %s", SDL_GetError());
        return false;
    }
    if ((desired.format != actual.format) || (desired.samples != actual.samples)) {
        SDL_Log("Could not get desired audio format");
        return false;
    }

    SDL_Log("-------------------");
    SDL_Log("sample rate: %d", actual.freq);
    SDL_Log("channels:    %d", actual.channels);
    SDL_Log("samples:     %d", actual.samples);
    SDL_Log("size:        %d", actual.size);
    SDL_Log("------------------");

    SDL_PauseAudioDevice(_audioDevice, 0);
    return true;
}

SDLWrapper::~SDLWrapper() {
    SDL_CloseAudioDevice(_audioDevice);
    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
    }
    if (_window) {
        SDL_DestroyWindow(_window);
    }
    SDL_Quit();
}
