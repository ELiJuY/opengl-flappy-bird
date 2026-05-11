#pragma once
#include <vector>

enum WallSegment
{
    SEG_NONE = -1,
    SEG_LEFT = 0,
    SEG_RIGHT = 1,
    SEG_BOTTOM = 2,
    SEG_TOP = 3
};

struct WallObstacle
{
    float z;
    float gapX;
    float gapY;

    bool scored;

    int colorIndex;
};

extern std::vector<WallObstacle> walls;

float generarGapX();
float generarGapY();
void inicializarParedes();

void recycleWalls();
void spawnWalls();