#include "world/sun.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "core/timing.h"
#include "game/player.h"
#include "graphics/sky.h"

glm::vec3 sunPosition(0.0f);
glm::vec3 sunDirection(0.0f);

void updateSun()
{
    float progress = getSunProgress();

    // =========================================
    // Noche
    // =========================================

    if (progress < 0.0f)
    {
        sunPosition = glm::vec3(0.0f, -999.0f, 0.0f);
        sunDirection = glm::vec3(0.0f, -1.0f, 0.0f);
        return;
    }

    // =========================================
    // Recorrido solar
    // =========================================

    float sunAngle =
        glm::mix(
            0.0f,
            glm::pi<float>(),
            progress
        );

    float radius = 40.0f;
    sunPosition.x = playerX;
    sunPosition.y = sin(sunAngle) * radius - 2.5f;
    sunPosition.z = playerZ - cos(sunAngle) * radius;

    // =========================================
    // Dirección solar
    // =========================================

    sunDirection =
        normalize(
            glm::vec3(
                0.0f,
                -sunPosition.y, -(sunPosition.z - playerZ)
            )
        );
}

float getSunProgress()
{

    // =========================================
    // Sol oculto
    // =========================================

    if (simulatedHour < SUNRISE_HOUR || simulatedHour > SUNSET_HOUR)
    {
        return -1.0f;
    }

    // =========================================
    // Progreso solar visible
    // =========================================

    return (simulatedHour - SUNRISE_HOUR) / (SUNSET_HOUR - SUNRISE_HOUR);
}