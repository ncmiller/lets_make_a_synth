#pragma once

#include <stdint.h>
#include <SDL.h>

class SDLWrapper {
public:
    SDLWrapper() = default;
    ~SDLWrapper();

    bool Init(
        const char* winTitle,
        uint32_t widthPx,
        uint32_t heightPx,
        uint32_t audioSampleRateHz,
        uint16_t audioSamplesPerBuffer,
        SDL_AudioCallback audioCallback,
        void* callbackUserdata);

    SDL_GLContext _gl_context = nullptr;
    SDL_Window* _window = nullptr;

private:
    bool InitWindow(const char* title, uint32_t widthPx, uint32_t heightPx);
    bool InitRenderer(uint32_t widthPx, uint32_t heightPx);
    bool InitAudio(uint32_t sampleRateHz, uint16_t samplesPerBuffer, SDL_AudioCallback audioCallback, void* callbackUserdata);

    SDL_AudioDeviceID _audioDevice;
};
