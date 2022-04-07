#include "audio.h"
#include "synth.h"
#include "utility.h"

namespace audio {

void AudioCallback(void* userdata, uint8_t* stream, int len) {
    Synth* synth = (Synth*)userdata;

    while (len > 0) {
        // TODO: loop over enabled oscillators
        // TODO: loop over oscillator voices

        float* left = (float*)(stream);
        float* right = (float*)(stream + sizeof(float));
        synth->osc.GetSample(left, right);
        *left *= MAX_VOLUME;
        *right *= MAX_VOLUME;

        // Advance forward in the stream
        stream += (2 * sizeof(float));
        len -= (2 * sizeof(float));
    }
}

} // namespace audio
