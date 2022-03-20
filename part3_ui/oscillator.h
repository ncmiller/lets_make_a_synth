#pragma once

typedef double (*OscillatorFn)(double t, double freqHz);
double sine(double t, double freqHz);
double square(double t, double freqHz);
double saw(double t, double freqHz);
double triangle(double t, double freqHz);
double whitenoise(double t, double freqHz);

class Oscillator {
public:
    void nextFn();
    double getSample(double t, double freqHz) const;

private:
    OscillatorFn _fn = sine;
};
