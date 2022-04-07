#pragma once

#include "constants.h"
#include <atomic>

struct Synth;

namespace oscillator {

typedef float (*Fn)(float phase);
float Sine(float phase);
float Square(float phase);
float Saw(float phase);
float Trianle(float phase);
float Whitenoise(float phase);

}

class Oscillator {
public:
    bool Init(Synth* synth);
    void NextFn();
    void GetSample(float* left, float* right);

    // Controllable from UI
    std::atomic<bool> enabled = true;
    std::atomic<oscillator::Fn> fn = oscillator::Sine;
    std::atomic<float> volume = 0.7f; // range [0, 1]
    std::atomic<float> pan = 0.0f; // range [-.5, .5]
    std::atomic<float> coarsePitch = 0.0f; // semitones, range [-36,36]
    std::atomic<float> finePitch = 0.0f; // cents, range [-100,100]
    std::atomic<bool> noteActive = false; // whether note should play
    std::atomic<uint8_t> noteIndex = 39; // C4, 0-based on 88-key piano, index 0 is note A0

private:
    float GetFrequency() const;

    static constexpr float A0Freq = 27.5f;

    Synth* _synth = nullptr;
    float _phase = 0.0f; // radians
};
