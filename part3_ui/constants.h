#pragma once

#include <stdint.h>
#include <math.h>

constexpr uint32_t WINDOW_WIDTH = 1280;
constexpr uint32_t WINDOW_HEIGHT = 720;
constexpr float SAMPLE_RATE_HZ = 48000.f;
constexpr float MAX_VOLUME = 0.1f; // about -20 dB
#ifdef IS_WASM_BUILD
constexpr uint16_t SAMPLES_PER_BUFFER = 256; // (256 / 48000) = 5.333 ms latency
#else
constexpr uint16_t SAMPLES_PER_BUFFER = 64; // (64 / 48000) = 1.333 ms latency
#endif

constexpr float TwoPi = 2.0f * (float)M_PI;
