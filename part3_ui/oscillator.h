#pragma once

#include "constants.h"
#include <atomic>
#include <array>

struct Synth;

namespace oscillator {

typedef float (*Fn)(float phase);
float Sine(float phase);
float Square(float phase);
float Saw(float phase);
float Triangle(float phase);
float Whitenoise(float phase);

}

class Oscillator {
public:
    struct Source {
        const char* name;
        oscillator::Fn fn;
    };

    bool Init(Synth* synth);
    void Prev();
    void Next();
    const char* GetName() const { return _sources[_sourceIndex].name; }
    float Fn(float phase) const { return _sources[_sourceIndex].fn(phase); }
    void GetSample(float* left, float* right);

    // Controllable from UI
    std::atomic<bool> enabled = true;
    std::atomic<float> volume = 0.7f; // range [0, 1]
    std::atomic<float> pan = 0.0f; // range [-.5, .5]
    std::atomic<float> coarsePitch = 0.0f; // semitones, range [-36,36]
    std::atomic<float> finePitch = 0.0f; // cents, range [-100,100]
    std::atomic<bool> noteActive = false; // whether note should play
    std::atomic<uint8_t> noteIndex = 39; // C3, 0-based on 88-key piano, index 0 is note A1

private:
    float GetFrequency() const;

    static constexpr float A0Freq = 27.5f;
    static constexpr std::array<Source, 5> _sources = {{
        { "Sine", oscillator::Sine },
        { "Square", oscillator::Square },
        { "Saw", oscillator::Saw },
        { "Triangle", oscillator::Triangle },
        { "Whitenoise", oscillator::Whitenoise },
    }};

    // Controllable from UI
    std::atomic<uint32_t> _sourceIndex = 0; // range [0, _sources.size() - 1]

    Synth* _synth = nullptr;
    float _phase = 0.0f; // radians
};
