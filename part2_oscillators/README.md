# Part 2 - Oscillators

In part 1, we got a basic sine wave at 440 Hz to play
out of the PC speakers. The sine wave is a type of
oscillator.

In part 2, we will explore other common synthesizer oscillators,
like the square wave, saw wave, and triangle wave, and a noise
generator.

## Square Wave

You probably already know what a square wave looks like, and you probably
already have a good idea of how to program one in software.

What you may not know (unless you remember your math classes) is that the
square wave has an interesting harmonic series consisting of a strong
fundamental frequency, and gradually weaker odd harmonics.

![Square Wave Harmonics](https://upload.wikimedia.org/wikipedia/commons/b/b5/Spectrum_square_oscillation.jpg)

By contrast, sine waves do not have harmonics. You just get the boring old
fundamental frequency. You can add multiple sine waves together to get a square wave.
As a result, the square wave has a more interesting and complex sound than
the smooth and clean sine wave. The square wave is often characterized as having a "rich" and
"buzzy" sound.

Let's change the sine wave to a square wave. In our audio callback, we can
disable the sine wave code and replace it with square wave code.

```c
    while (len > 0) {
        double y = 0.0;
        if (_playSound) {
#if 0
            // Equation for a sine wave:
            //    y(t) = A * sin(2 * PI * f * t + shift)
            y = maxAmp * sin(twoPi * freqHz * t);
#endif
            // Square wave
            double dutyCycle = 0.5;
            if (t < (periodS * dutyCycle)) {
                y = maxAmp;
            } else {
                y = -maxAmp;
            }
        }
```

The frequency here is still 440 Hz. The period of the wave is
1 / 440 = 2.27272 ms. The square wave is `+maxAmp` for the first
half of the period, and `-maxAmp` for the second half, then the wave
repeats.

The transition from high to low is controlled by the `dutyCycle` variable.
By setting the duty cycle to a small number, like 0.1, you get a very
different sound.

## Switching between sounds

It would be nice if we could switch between sine, square, and other types
of oscillators without having to recompile the code every time.

Let's try this - whenever the right mouse button is click, we will cycle
to the next oscillator.

(Refactor, functions for each wave)

## Saw Wave
## Triangle Wave
## Noise Generator
