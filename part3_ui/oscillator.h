#pragma once

#include <atomic>

struct Synth;

typedef double (*OscillatorFn)(double t, double freqHz);
double sine(double t, double freqHz);
double square(double t, double freqHz);
double saw(double t, double freqHz);
double triangle(double t, double freqHz);
double whitenoise(double t, double freqHz);

class Oscillator {
public:
    bool init(Synth* synth);
    void nextFn();
    double getSample(double t, double freqHz) const;

    std::atomic<float> volume = 0.7f; // range [0, 1]
    std::atomic<float> pan = 0.0f; // range [-.5, .5]

private:
    Synth* _synth = nullptr;
    OscillatorFn _fn = sine;
};
