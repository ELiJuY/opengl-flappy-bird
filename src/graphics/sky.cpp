#include <graphics/sky.h>

#include "core/frame.h"
#include "core/timing.h"
#include "glad/glad.h"
#include "glm/common.hpp"

float turbulenceStart = 0.7f;
float turbulenceEnd = 0.9f;

glm::vec3 dawnSky   = glm::vec3(1.0f, 0.55f, 0.25f);
glm::vec3 daySky    = glm::vec3(0.35f, 0.65f, 1.0f);
glm::vec3 sunsetSky = glm::vec3(1.0f, 0.35f, 0.15f);
glm::vec3 nightSky = glm::vec3(0.003f,0.003f,0.015f);

glm::vec3 dawnLight   = glm::vec3(1.0f, 0.7f, 0.45f);
glm::vec3 dayLight    = glm::vec3(1.0f, 0.95f, 0.85f);
glm::vec3 sunsetLight = glm::vec3(1.0f, 0.5f, 0.25f);
glm::vec3 nightLight  = glm::vec3(0.12f, 0.18f, 0.35f);

glm::vec3 calculateSkyLightColor(float hour)
{
    // =========================================
    // Día estable
    // =========================================

    if (hour >= 8.0f && hour < 17.5f)
    {
        return daySky;
    }

    // =========================================
    // Atardecer azul -> naranja
    // =========================================

    if (hour >= 17.5f && hour < 18.0f)
    {
        float t = (hour - 17.5f) / 0.5f;
        return mix(daySky, sunsetSky, t);
    }

    // =========================================
    // Atardecer naranja -> noche
    // =========================================

    if (hour >= 18.0f && hour < 19.0f)
    {
        float t = (hour - 18.0f);

        return mix(sunsetSky, nightSky, t);
    }

    // =========================================
    // Noche estable
    // =========================================

    if (hour >= 19.0f || hour < 6.0f)
    {
        return nightSky;
    }

    // =========================================
    // Amanecer noche -> naranja
    // =========================================

    if (hour >= 6.0f && hour < 7.0f)
    {
        float t = hour - 6.0f;
        return mix(nightSky, dawnSky, t);
    }

    // =========================================
    // Amanecer naranja -> día
    // =========================================

    float t = hour - 7.0f;
    return mix(dawnSky, daySky, t);
}


void updateSky()
{

    glm::vec3 skyColor = calculateSkyLightColor(simulatedHour);
    glClearColor(
        skyColor.r,
        skyColor.g,
        skyColor.b,
        1.0f
    );
}



