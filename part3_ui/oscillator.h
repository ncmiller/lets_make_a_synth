#pragma once

#include "constants.h"
#include <atomic>

struct Synth;

typedef float (*OscillatorFn)(float phase);
float sine(float phase);
float square(float phase);
float saw(float phase);
float triangle(float phase);
float whitenoise(float phase);

class Oscillator {
public:
    bool init(Synth* synth);
    void nextFn();
    float getFrequency() const;
    float getSample();

    std::atomic<bool> enabled = true;
    std::atomic<OscillatorFn> fn = sine;
    std::atomic<float> volume = 0.7f; // range [0, 1]
    std::atomic<float> pan = 0.0f; // range [-.5, .5]
    std::atomic<float> coarsePitch = 0.0f; // semitones, range [-36,36]
    std::atomic<float> finePitch = 0.0f; // cents, range [-100,100]
    std::atomic<bool> noteActive = false; // whether note should play
    std::atomic<uint8_t> noteIndex = 39; // C4, 0-based on 88-key piano, index 0 is note A0

private:
    static constexpr float A0Freq = 27.5f;

    Synth* _synth = nullptr;
    float _phase = 0.0f; // radians
};
