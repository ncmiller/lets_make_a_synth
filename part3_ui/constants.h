#pragma once

#define WINDOW_WIDTH 360
#define WINDOW_HEIGHT 360
#define DEFAULT_FREQ 220.0

// Audio Configuration
#define SAMPLE_RATE_HZ 48000
#define NUM_SOUND_CHANNELS 2
#define SAMPLE_FORMAT AUDIO_F32SYS // 32-bit float
#define VOLUME 0.025 // max volume, about -32 dB
#ifdef IS_WASM_BUILD
#define SAMPLES_PER_BUFFER 256 // (256 / 48000) = 5.333 ms latency
#else
#define SAMPLES_PER_BUFFER 64 // (64 / 48000) = 1.333 ms latency
#endif

constexpr double dt = 1.0 / SAMPLE_RATE_HZ;

