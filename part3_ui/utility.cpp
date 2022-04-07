#include "utility.h"
#include <algorithm>

namespace utility {

float Map(float value, float start1, float end1, float start2, float end2) {
    float normalizedValue = (value - start1) / (end1 - start1);
    return (start2 + normalizedValue * (end2 - start2));
}

float Clamp(float value, float minValue, float maxValue) {
    return std::min(std::max(minValue, value), maxValue);
}

} // namespace utility
