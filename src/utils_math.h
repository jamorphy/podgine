#ifndef UTILS_MATH_H
#define UTILS_MATH_H

#define TO_RAD(deg) ((deg) * M_PI / 180.0f)

float clamp(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

#endif
