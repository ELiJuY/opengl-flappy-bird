#include "core/timing.h"

#include <core/debug.h>
#include <cmath>

bool keyA = false;
bool keyD = false;
bool keyF = false;

float currentTime = 0.0f;
float deltaTime = 0.0f;
float lastTime = 0.0f;
float nightFactor = getNightFactor();
float simulatedHour = 12.0f;
#ifdef DEBUG_SOLAR
float dayDuration = 30.0f;
#else
float dayDuration = 120.0f;
#endif

bool isNight()
{
    return simulatedHour >= 19.0f || simulatedHour < 6.0f;
}

bool isDay()
{
    return
        simulatedHour >= 8.0f &&
        simulatedHour < 17.5f;
}

bool isSunset()
{
    return
        simulatedHour >= 17.5f &&
        simulatedHour < 19.0f;
}

bool isDawn()
{
    return
        simulatedHour >= 6.0f &&
        simulatedHour < 8.0f;
}

float getNightFactor()
{
    // =========================================
    // Noche completa
    // =========================================

    if (simulatedHour >= 19.0f ||
        simulatedHour < 6.0f)
    {
        return 1.0f;
    }

    // =========================================
    // Sunset
    // =========================================

    if (simulatedHour >= 18.0f &&
        simulatedHour < 19.0f)
    {
        return simulatedHour - 18.0f;
    }

    // =========================================
    // Amanecer
    // =========================================

    if (simulatedHour >= 6.0f &&
        simulatedHour < 7.0f)
    {
        return 1.0f - (simulatedHour - 6.0f);
    }

    // =========================================
    // Día
    // =========================================

    return 0.0f;
}

void updateGameTime()
{
    float normalizedTime = fmod(currentTime, dayDuration) / dayDuration;
    simulatedHour = 12.0f + normalizedTime * 24.0f;

    if (simulatedHour >= 24.0f)
    {
        simulatedHour -= 24.0f;
    }
}