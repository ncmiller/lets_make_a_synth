#pragma once

#include <stdint.h>

namespace audio {
void AudioCallback(void* userdata, uint8_t* stream, int len);
} // namespace audio
