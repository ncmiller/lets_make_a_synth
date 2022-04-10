#include "oscillator.h"
#include "utility.h"
#include <math.h>
#include <stdlib.h>
#include <SDL.h>

namespace oscillator {

float Sine(float phase) {
    return sin(phase);
}

float Square(float phase) {
    return (phase < M_PI ? 1 : -1);
}

float Saw(float phase) {
    return utility::Map(phase, 0.f, TWOPI, -1.f, 1.f);
}

float Triangle(float phase) {
    if (phase < M_PI) {
        return utility::Map(phase, 0.f, (float)M_PI, 1.f, -1.f);
    } else {
        return utility::Map(phase, (float)M_PI, TWOPI, -1.f, 1.f);
    }
}

float Whitenoise(float phase) {
    // phase unused
    return utility::Map((float)rand(), 0.f, (float)RAND_MAX, -1.f, 1.f);
}

} // namespace oscillator

bool Oscillator::Init(Synth* synth) {
    _synth = synth;
    return true;
}

void Oscillator::Prev() {
    _sourceIndex = (_sourceIndex == 0 ? _sources.size() - 1: _sourceIndex - 1);
}

void Oscillator::Next() {
    _sourceIndex = (_sourceIndex + 1) % _sources.size();
}

// See this page for converting notes -> cents -> frequency
// https://en.wikipedia.org/wiki/Cent_(music)
float Oscillator::GetFrequency() const {
    float cents = noteIndex * 100.0f;
    cents += (round(coarsePitch) * 100.0f);
    cents += finePitch;
    return A0Freq * pow(2.f, cents / 1200.f);
}

void Oscillator::GetSample(float* left, float* right) {
    float sample = _sources[_sourceIndex].fn(_phase);

    // Update phase for next time
    float dPhase = TWOPI * GetFrequency() / SAMPLE_RATE_HZ;
    _phase = fmodf(_phase + dPhase, TWOPI);

    if (!noteActive) {
        *left = 0.0f;
        *right = 0.0f;
        return;
    }

    *left = sample * volume;
    *right = sample * volume;

    // Constant power panning
    float theta = utility::Map(pan, -.5f, .5f, 0.f, (float)M_PI / 2.f);
    *left *= cos(theta);
    *right *= sin(theta);
}
