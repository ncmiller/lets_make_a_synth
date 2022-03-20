#pragma once

namespace Oscillator {

typedef double (*Fn)(double t, double freqHz);
double sine(double t, double freqHz);
double square(double t, double freqHz);
double saw(double t, double freqHz);
double triangle(double t, double freqHz);
double whitenoise(double t, double freqHz);

struct Context {
    Fn fn = sine;
};

void nextFn(Context& context);
double getSample(const Context& context, double t, double freqHz);

} // namespace Oscillator
