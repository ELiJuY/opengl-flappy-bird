#include "core/mathUtils.h"

#include <cstdlib>

float clampf(float value, float minv, float maxv)
{
    if (value < minv) return minv;
    if (value > maxv) return maxv;
    return value;
}

float randomFloat(float minValue, float maxValue)
{
    float r = (float)rand() / (float)RAND_MAX;

    return minValue + r * (maxValue - minValue);
}