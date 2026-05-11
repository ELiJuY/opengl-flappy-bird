#include "graphics/camera.h"

#include <algorithm>
#include <cmath>

#include "core/config.h"
#include "core/frame.h"
#include "core/debug.h"
#include "core/timing.h"
#include "game/player.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "world/walls.h"
#include "world/world.h"

glm::vec3 pos_obs = glm::vec3(0.0f, 0.0f, 8.0f);
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

#ifndef DEBUG_CAMERA
float fov = 35.0f;
#else
float fov = 100.0f;
#endif
float aspect = 4.0f / 3.0f;
float nearPlane = 0.1f;
float farPlane  = 80.0f;

namespace {
float currentCameraDistance = 4.5f;

struct CameraOccluderBox
{
    glm::vec3 min;
    glm::vec3 max;
};

bool segmentIntersectsAabb(
    glm::vec3 start,
    glm::vec3 end,
    const CameraOccluderBox& box
)
{
    glm::vec3 direction = end - start;
    float tMin = 0.0f;
    float tMax = 1.0f;

    for (int axis = 0; axis < 3; ++axis)
    {
        float origin = start[axis];
        float delta = direction[axis];
        float minValue = box.min[axis];
        float maxValue = box.max[axis];

        if (std::abs(delta) < 0.0001f)
        {
            if (origin < minValue || origin > maxValue)
            {
                return false;
            }

            continue;
        }

        float invDelta = 1.0f / delta;
        float t1 = (minValue - origin) * invDelta;
        float t2 = (maxValue - origin) * invDelta;

        if (t1 > t2)
        {
            std::swap(t1, t2);
        }

        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);

        if (tMin > tMax)
        {
            return false;
        }
    }

    return true;
}

void addObstacleBoxes(
    const WallObstacle& wall,
    CameraOccluderBox boxes[4]
)
{
    float wallHalfZ = wallThickness * 0.5f;
    float leftEdge = wall.gapX - holeHalfW;
    float rightEdge = wall.gapX + holeHalfW;
    float bottomEdge = wall.gapY - holeHalfH;
    float topEdge = wall.gapY + holeHalfH;
    float obstacleBottom = -worldHalfY - 0.12f;
    float obstacleTop = worldHalfY;

    boxes[0] = {
        glm::vec3(-worldHalfX, obstacleBottom, wall.z - wallHalfZ),
        glm::vec3(leftEdge, obstacleTop, wall.z + wallHalfZ)
    };

    boxes[1] = {
        glm::vec3(rightEdge, obstacleBottom, wall.z - wallHalfZ),
        glm::vec3(worldHalfX, obstacleTop, wall.z + wallHalfZ)
    };

    boxes[2] = {
        glm::vec3(leftEdge, obstacleBottom, wall.z - wallHalfZ),
        glm::vec3(rightEdge, bottomEdge, wall.z + wallHalfZ)
    };

    boxes[3] = {
        glm::vec3(leftEdge, topEdge, wall.z - wallHalfZ),
        glm::vec3(rightEdge, obstacleTop, wall.z + wallHalfZ)
    };
}

float calculateCameraDistance(
    const glm::vec3& desiredCameraPos,
    const glm::vec3& cameraTarget
)
{
    float targetDistance = 4.5f;

    for (const WallObstacle& wall : walls)
    {
        if (wall.z >= playerZ)
        {
            continue;
        }

        if (wall.z < playerZ - 5.2f)
        {
            continue;
        }

        CameraOccluderBox boxes[4];
        addObstacleBoxes(wall, boxes);

        for (const CameraOccluderBox& box : boxes)
        {
            if (!segmentIntersectsAabb(desiredCameraPos, cameraTarget, box))
            {
                continue;
            }

            float distanceToPassObstacle =
                playerZ - (wall.z + wallThickness * 0.5f) - 0.35f;

            targetDistance =
                std::min(
                    targetDistance,
                    glm::clamp(distanceToPassObstacle, 1.55f, 4.5f)
                );
        }
    }

    return targetDistance;
}
}

void updateCamera(glm::mat4& P, glm::mat4& V) {
    aspect = (float) ANCHO / (float) ALTO;

#ifndef DEBUG_CAMERA
    float cameraY = std::min(playerY + 1.2f, 2.2f);
    target = glm::vec3(playerX, playerY + 0.15f, playerZ + 3.2f);

    glm::vec3 desiredCameraPos(playerX, cameraY, playerZ - 4.5f);

    float targetCameraDistance =
        calculateCameraDistance(
            desiredCameraPos,
            glm::vec3(playerX, playerY, playerZ)
        );

    float smoothing =
        1.0f - std::exp(-deltaTime * 12.0f);

    currentCameraDistance =
        glm::mix(
            currentCameraDistance,
            targetCameraDistance,
            glm::clamp(smoothing, 0.0f, 1.0f)
        );

    pos_obs = glm::vec3(playerX, cameraY, playerZ - currentCameraDistance);
#else

    //Órbita
    pos_obs = glm::vec3(
        45.0f,
        -10.0f,
        playerZ
    );

    target = glm::vec3(
        0.0f,
        0.0f,
        playerZ
    );

    //Sombras

#endif

    P = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    V = lookAt(pos_obs, target, up);
}
