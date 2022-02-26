#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>

// Window Configuration
#define WINDOW_WIDTH 360
#define WINDOW_HEIGHT 640

// Audio Configuration
#define SAMPLE_RATE_HZ 44100
#define SAMPLE_FORMAT AUDIO_S16SYS // Signed 16-bit samples, in system byte order
#define NUM_SOUND_CHANNELS 2 // stereo
#define SAMPLE_BUFFER_SIZE 1024 // smaller == lower latency, but too low will cause crackling

static SDL_Renderer* _renderer = nullptr;
static SDL_Window* _window = nullptr;

static void close(void) {
    SDL_Log("Closing");
    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
    }
    if (_window) {
        SDL_DestroyWindow(_window);
    }
    Mix_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    // Initialize SDL2_mixer
    if (0 != Mix_OpenAudio(SAMPLE_RATE_HZ, SAMPLE_FORMAT, NUM_SOUND_CHANNELS, SAMPLE_BUFFER_SIZE)) {
        SDL_Log("Mix_OpenAudio failed, error: %s", Mix_GetError());
        return -2;
    }

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

    // Configure SDL_mixer
    constexpr uint8_t volume = MIX_MAX_VOLUME / 2;
    Mix_AllocateChannels(16);
    Mix_VolumeMusic(volume);
    Mix_Volume(-1 /* apply to all channels */, volume);

    // Main SDL Loop
    bool loopShouldStop = false;
    SDL_Log("Starting main loop");
    while (!loopShouldStop) {
        // Check for events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                loopShouldStop = true;
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
