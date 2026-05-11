#include <cstdlib>
#include "world/walls.h"
#include "game/player.h"
#include "world/world.h"

std::vector<WallObstacle> walls;

namespace {
const float WALL_RECYCLE_DISTANCE_BEHIND_PLAYER = 12.0f;
}

float generarGapY()
{
    float rango = (worldHalfY - holeHalfH - 0.2f);
    return -rango + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (2.0f * rango);
}

float generarGapX()
{
    float rango = (xLimit - holeHalfW - 0.15f);
    return -rango + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (2.0f * rango);
}

void inicializarParedes()
{
    walls.clear();

    float firstZ = playerZ + 9.5f;
    int totalWalls = minWallsAhead + 1;

    for (int i = 0; i < totalWalls; ++i)
    {
        WallObstacle w;
        w.z = firstZ + i * obstacleSpacing;
        w.gapX = generarGapX();
        w.gapY = generarGapY();
        w.scored = false;
        w.colorIndex = i % 3;
        walls.push_back(w);
    }
}

void recycleWalls() {
    while (
        !walls.empty() &&
        playerZ > walls.front().z + wallThickness + WALL_RECYCLE_DISTANCE_BEHIND_PLAYER
    ) {
        walls.erase(walls.begin());
    }
}

void spawnWalls() {
    float desiredLastZ =
        playerZ + 9.5f + minWallsAhead * obstacleSpacing;

    while (walls.empty() || walls.back().z < desiredLastZ) {
        WallObstacle w;

        float lastZ =
                walls.empty()
                    ? (playerZ + 9.5f)
                    : walls.back().z;

        w.z = lastZ + obstacleSpacing;
        w.gapX = generarGapX();
        w.gapY = generarGapY();
        w.scored = false;

        if (walls.empty())
        {
            w.colorIndex = 0;
        }
        else
        {
            w.colorIndex =
                (walls.back().colorIndex + 1) % 3;
        }

        walls.push_back(w);
    }
}




