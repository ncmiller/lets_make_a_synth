#include "sdlwrapper.h"
#include <glad/glad.h>

#define SAMPLE_FORMAT AUDIO_F32SYS // 32-bit float
#define NUM_SOUND_CHANNELS 2

#define RETURN_FALSE_IF_FALSE(expr) if ((!expr)) { return false; }

bool SDLWrapper::init(
        const char* winTitle,
        uint32_t widthPx,
        uint32_t heightPx,
        uint32_t audioSampleRateHz,
        uint16_t audioSamplesPerBuffer,
        SDL_AudioCallback audioCallback,
        void* callbackUserdata) {
    RETURN_FALSE_IF_FALSE(initWindow(winTitle, widthPx, heightPx));
    RETURN_FALSE_IF_FALSE(initRenderer(widthPx, heightPx));
    RETURN_FALSE_IF_FALSE(initAudio(audioSampleRateHz, audioSamplesPerBuffer, audioCallback, callbackUserdata));
    return true;
}

bool SDLWrapper::initWindow(const char* title, uint32_t widthPx, uint32_t heightPx) {
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    // CONFIGURE OPENGL ATTRIBUTES USING SDL:
    int context_flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#ifdef _DEBUG
    context_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    _window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        (int)widthPx, (int)heightPx,
        SDL_WINDOW_OPENGL
    );
    if (_window == nullptr) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool SDLWrapper::initRenderer(uint32_t widthPx, uint32_t heightPx) {
    _gl_context = SDL_GL_CreateContext(_window);
    if (_gl_context == nullptr) {
        SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
        return false;
    }
    SDL_GL_MakeCurrent(_window, _gl_context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to initialize GLAD");
        return false;
    }

    SDL_Log("-------------------");
    SDL_Log("OpenGL Version: %d.%d", GLVersion.major, GLVersion.minor);
    SDL_Log("OpenGL Shading Language Version: %s", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    SDL_Log("OpenGL Vendor: %s", (char*)glGetString(GL_VENDOR));
    SDL_Log("OpenGL Renderer: %s", (char*)glGetString(GL_RENDERER));
    return true;
}

bool SDLWrapper::initAudio(uint32_t sampleRateHz, uint16_t samplesPerBuffer, SDL_AudioCallback audioCallback, void* callbackUserdata) {
    // Initialize Audio
    SDL_AudioSpec desired = {};
    desired.freq = (int)sampleRateHz;
    desired.format = SAMPLE_FORMAT;
    desired.channels = NUM_SOUND_CHANNELS;
    desired.samples = samplesPerBuffer;
    desired.callback = audioCallback;
    desired.userdata = callbackUserdata;

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
    if (_gl_context) {
        SDL_GL_DeleteContext(_gl_context);
    }
    if (_window) {
        SDL_DestroyWindow(_window);
    }
    SDL_Quit();
}
