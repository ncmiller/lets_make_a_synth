#include "synth.h"
#include "audio.h"
#if IS_WASM_BUILD
#include <emscripten.h>
#endif

#define RETURN_1_IF_FALSE(expr) if ((!expr)) { return 1; }

static void LoopOnce(void* arg) {
    Synth* synth = (Synth*)arg;
    synth->input.PollEvents();
    synth->ui.Draw();
    SDL_GL_SwapWindow(synth->sdl._window);
}

int main(int argc, char* argv[]) {
    auto synth = std::make_unique<Synth>();

    RETURN_1_IF_FALSE(synth->sdl.Init(
            "Synth (part 3)",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            (uint32_t)SAMPLE_RATE_HZ,
            SAMPLES_PER_BUFFER,
            audio::AudioCallback,
            (void*)synth.get()));
    RETURN_1_IF_FALSE(synth->input.Init(synth.get()));
    RETURN_1_IF_FALSE(synth->osc.Init(synth.get()));
    RETURN_1_IF_FALSE(synth->ui.Init(synth.get()));

#ifdef IS_WASM_BUILD
    emscripten_set_main_loop_arg(LoopOnce, synth.get(), 60, 1);
#else
    SDL_Log("Starting main loop");
    while (synth->running) {
        LoopOnce(synth.get());
    }
#endif

    SDL_Log("Exiting");
    return 0;
}
