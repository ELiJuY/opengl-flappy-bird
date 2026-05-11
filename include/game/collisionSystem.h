#pragma once
#include "world/walls.h"

struct CollisionInfo
{
    bool hit;

    int wallIndex;

    WallSegment segment;
};

CollisionInfo detectCollision();