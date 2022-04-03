#pragma once

namespace utility {

// Remap a value from range 1 to range 2
float map(float value, float start1, float end1, float start2, float end2);

// Clamp a value between minValue and maxValue
float clamp(float value, float minValue, float maxValue);

} // namespace utility
