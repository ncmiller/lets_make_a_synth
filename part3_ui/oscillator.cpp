#include "oscillator.h"
#include "utility.h"
#include <math.h>
#include <stdlib.h>

float sine(float phase) {
    return sin(phase);
}

float square(float phase) {
    return (phase < M_PI ? 1 : -1);
}

float saw(float phase) {
    return utility::map(phase, 0.f, TwoPi, -1.f, 1.f);
}

float triangle(float phase) {
    if (phase < M_PI) {
        return utility::map(phase, 0.f, (float)M_PI, 1.f, -1.f);
    } else {
        return utility::map(phase, (float)M_PI, TwoPi, -1.f, 1.f);
    }
}

float whitenoise(float phase) {
    // phase unused
    return utility::map((float)rand(), 0.f, (float)RAND_MAX, -1.f, 1.f);
}

bool Oscillator::init(Synth* synth) {
    _synth = synth;
    return true;
}

void Oscillator::nextFn() {
    if (fn == sine) {
        fn = square;
    } else if (fn == square) {
        fn = triangle;
    } else if (fn == triangle) {
        fn = saw;
    } else if (fn == saw) {
        fn = whitenoise;
    } else if (fn == whitenoise) {
        fn = sine;
    }
}

// See this page for converting notes -> cents -> frequency
// https://en.wikipedia.org/wiki/Cent_(music)
float Oscillator::getFrequency() const {
    float cents = noteIndex * 100.0f;
    cents += (coarsePitch * 100.0f);
    cents += finePitch;
    return A0Freq * pow(2.f, cents / 1200.f);
}

float Oscillator::getSample() {
    float sample = fn.load()(_phase);
    float dPhase = TwoPi * getFrequency() / SAMPLE_RATE_HZ;
    _phase = fmodf(_phase + dPhase, TwoPi);
    return (noteActive.load() ? sample : 0.0f);
}
