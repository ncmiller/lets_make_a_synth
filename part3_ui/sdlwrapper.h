#pragma once

#include <stdint.h>
#include <SDL.h>

class SDLWrapper {
public:
    SDLWrapper() = default;
    ~SDLWrapper();

    bool init(
        const char* winTitle,
        uint32_t widthPx,
        uint32_t heightPx,
        uint32_t audioSampleRateHz,
        uint16_t audioSamplesPerBuffer,
        SDL_AudioCallback audioCallback);

    SDL_GLContext _gl_context = nullptr;
    SDL_Window* _window = nullptr;

private:
    bool initWindow(const char* title, uint32_t widthPx, uint32_t heightPx);
    bool initRenderer(uint32_t widthPx, uint32_t heightPx);
    bool initAudio(uint32_t sampleRateHz, uint16_t samplesPerBuffer, SDL_AudioCallback audioCallback);

    SDL_AudioDeviceID _audioDevice;
};
