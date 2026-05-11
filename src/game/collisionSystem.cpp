#include "game/collisionSystem.h"

#include <array>
#include <cfloat>

#include "game/player.h"
#include "glm/glm.hpp"
#include "world/walls.h"
#include "world/world.h"

namespace {
struct SphereCollider
{
    glm::vec3 center;
    float radius;
};

struct AabbCollider
{
    glm::vec3 min;
    glm::vec3 max;
    WallSegment segment;
};

constexpr float PLAYER_MODEL_SCALE = 0.90f;

std::array<SphereCollider, 5> getPlayerColliderSpheres()
{
    glm::mat4 M = calcularMatrizJugador();

    std::array<SphereCollider, 5> localSpheres =
    {
        SphereCollider { glm::vec3( 0.00f, 0.00f,  0.04f), 0.145f },
        SphereCollider { glm::vec3( 0.00f, 0.00f,  0.30f), 0.105f },
        SphereCollider { glm::vec3( 0.00f, 0.00f, -0.30f), 0.085f },
        SphereCollider { glm::vec3(-0.28f, 0.00f,  0.02f), 0.105f },
        SphereCollider { glm::vec3( 0.28f, 0.00f,  0.02f), 0.105f }
    };

    for (SphereCollider& sphere : localSpheres)
    {
        sphere.center = glm::vec3(M * glm::vec4(sphere.center, 1.0f));
        sphere.radius *= PLAYER_MODEL_SCALE;
    }

    return localSpheres;
}

bool sphereIntersectsAabb(const SphereCollider& sphere, const AabbCollider& box)
{
    glm::vec3 closest = glm::clamp(sphere.center, box.min, box.max);
    glm::vec3 delta = sphere.center - closest;

    return glm::dot(delta, delta) <= sphere.radius * sphere.radius;
}

float penetrationAgainstAabb(const SphereCollider& sphere, const AabbCollider& box)
{
    glm::vec3 closest = glm::clamp(sphere.center, box.min, box.max);
    glm::vec3 delta = sphere.center - closest;
    float distance = glm::length(delta);

    return sphere.radius - distance;
}

CollisionInfo detectWorldBoundsCollision(const std::array<SphereCollider, 5>& spheres)
{
    for (const SphereCollider& sphere : spheres)
    {
        if (sphere.center.y - sphere.radius < -worldHalfY)
        {
            return { true, -1, SEG_BOTTOM };
        }

        if (sphere.center.y + sphere.radius > worldHalfY)
        {
            return { true, -1, SEG_TOP };
        }

        if (sphere.center.x - sphere.radius < -worldHalfX)
        {
            return { true, -1, SEG_LEFT };
        }

        if (sphere.center.x + sphere.radius > worldHalfX)
        {
            return { true, -1, SEG_RIGHT };
        }
    }

    return { false, -1, SEG_NONE };
}

std::array<AabbCollider, 4> buildObstacleBoxes(const WallObstacle& wall)
{
    float wallHalfZ = wallThickness * 0.5f;
    float leftEdge = wall.gapX - holeHalfW;
    float rightEdge = wall.gapX + holeHalfW;
    float bottomEdge = wall.gapY - holeHalfH;
    float topEdge = wall.gapY + holeHalfH;

    return
    {
        AabbCollider {
            glm::vec3(-worldHalfX, -worldHalfY, wall.z - wallHalfZ),
            glm::vec3(leftEdge, worldHalfY, wall.z + wallHalfZ),
            SEG_LEFT
        },
        AabbCollider {
            glm::vec3(rightEdge, -worldHalfY, wall.z - wallHalfZ),
            glm::vec3(worldHalfX, worldHalfY, wall.z + wallHalfZ),
            SEG_RIGHT
        },
        AabbCollider {
            glm::vec3(leftEdge, -worldHalfY, wall.z - wallHalfZ),
            glm::vec3(rightEdge, bottomEdge, wall.z + wallHalfZ),
            SEG_BOTTOM
        },
        AabbCollider {
            glm::vec3(leftEdge, topEdge, wall.z - wallHalfZ),
            glm::vec3(rightEdge, worldHalfY, wall.z + wallHalfZ),
            SEG_TOP
        }
    };
}
}

CollisionInfo detectCollision()
{
    std::array<SphereCollider, 5> playerSpheres = getPlayerColliderSpheres();

    CollisionInfo boundsHit = detectWorldBoundsCollision(playerSpheres);

    if (boundsHit.hit)
    {
        return boundsHit;
    }

    for (size_t i = 0; i < walls.size(); ++i)
    {
        const WallObstacle& w = walls[i];

        std::array<AabbCollider, 4> obstacleBoxes = buildObstacleBoxes(w);
        float deepestPenetration = -FLT_MAX;
        WallSegment seg = SEG_NONE;

        for (const SphereCollider& sphere : playerSpheres)
        {
            if (
                sphere.center.z + sphere.radius < w.z - wallThickness * 0.5f ||
                sphere.center.z - sphere.radius > w.z + wallThickness * 0.5f
            )
            {
                continue;
            }

            for (const AabbCollider& box : obstacleBoxes)
            {
                if (!sphereIntersectsAabb(sphere, box))
                {
                    continue;
                }

                float penetration = penetrationAgainstAabb(sphere, box);

                if (penetration > deepestPenetration)
                {
                    deepestPenetration = penetration;
                    seg = box.segment;
                }
            }
        }

        if (seg != SEG_NONE)
        {
            return { true, (int)i, seg };
        }
    }

    return { false, -1, SEG_NONE };
}
