#ifndef mathutils_h
#define mathutils_h

#include <cstdlib>

constexpr inline float clamp(float min, float max, float val) {
    return (val < min) ? min : ((val > max) ? max : val);
}

constexpr inline float clamp01(float val) {
    return (val < 0) ? 0 : ((val > 1) ? 1 : val);
}

#endif