#include "audio.h"
#include "synth.h"
#include "utility.h"

namespace audio {

void AudioCallback(void* userdata, uint8_t* stream, int len) {
    Synth* synth = (Synth*)userdata;

    // TODO: loop over enabled oscillators
    // TODO: loop over oscillator voices
    float volume = synth->osc.volume.load();
    float pan = synth->osc.pan.load();
    float noteActive = synth->osc.noteActive.load();

    float leftWeight = utility::map(pan, -.5f, .5f, 1.f, 0.f);
    float rightWeight = 1.f - leftWeight;

    while (len > 0) {
        float y = MAX_VOLUME * volume * synth->osc.getSample();

        // Populate left and right channels based on pan
        float* left = (float*)(stream);
        float* right = (float*)(stream + 4);
        *left = (float)y * leftWeight;
        *right = (float)y * rightWeight;

        // Advance forward in the stream
        stream += (2 * sizeof(float));
        len -= (2 * sizeof(float));
    }
}

} // namespace audio
