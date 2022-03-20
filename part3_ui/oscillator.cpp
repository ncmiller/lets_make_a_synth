#include "oscillator.h"
#include <math.h>
#include <stdlib.h>

double sine(double t, double freqHz) {
    t = fmod(t, 1.0 / freqHz);
    constexpr double twoPi = 2.0 * M_PI;
    return sin(twoPi * freqHz * t);
}

double square(double t, double freqHz) {
    t = fmod(t, 1.0 / freqHz);
    const double halfPeriodS = 1.0 / freqHz / 2.0;
    if (t < halfPeriodS) {
        return 1;
    } else {
        return -1;
    }
}

double saw(double t, double freqHz) {
    t = fmod(t, 1.0 / freqHz);
    const double periodS = 1.0 / freqHz;
    const double percentComplete = t / periodS;
    // scale and offset to get it in range [-1, 1]
    return 2.0 * percentComplete - 1.0;
}

double triangle(double t, double freqHz) {
    t = fmod(t, 1.0 / freqHz);
    const double periodS = 1.0 / freqHz;
    const double percentComplete = t / periodS;
    if (percentComplete <= 0.5) {
        // 1 down to -1
        return 1.0 - 4.0 * percentComplete;
    } else {
        // -1 up to 1
        return -1.0 + 4.0 * (percentComplete - 0.5);
    }
}

double whitenoise(double t, double freqHz) {
    double rand_normalized = (double)rand() / (double)RAND_MAX;
    return 2.0 * rand_normalized - 1.0;
}

void Oscillator::nextFn() {
    if (_fn == sine) {
        _fn = square;
    } else if (_fn == square) {
        _fn = triangle;
    } else if (_fn == triangle) {
        _fn = saw;
    } else if (_fn == saw) {
        _fn = whitenoise;
    } else if (_fn == whitenoise) {
        _fn = sine;
    }
}

double Oscillator::getSample(double t, double freqHz) const {
    return _fn(t, freqHz);
}
