#include "graphics/worldRenderer.h"

#include "core/timing.h"
#include "game/gameState.h"
#include "game/player.h"
#include "graphics/camera.h"
#include "graphics/meshes.h"
#include "graphics/renderer.h"
#include "world/sun.h"
#include "world/walls.h"
#include "world/world.h"

namespace {
float floorCenterY()
{
    return -worldHalfY - FLOOR_THICKNESS * 0.5f;
}

float worldHeight()
{
    return worldHalfY * 2.0f;
}

float obstacleBottomY()
{
    return -worldHalfY - FLOOR_THICKNESS * 0.6f;
}

float obstacleTopY()
{
    return worldHalfY;
}

float obstacleFullHeight()
{
    return obstacleTopY() - obstacleBottomY();
}

float obstacleFullCenterY()
{
    return (obstacleTopY() + obstacleBottomY()) * 0.5f;
}
}

void renderFloor(mat4 P, mat4 V) {


    // Segmento actual basado en la posición del jugador
    int currentFloor = (int)floor(playerZ / FLOOR_LENGTH);

    for (int i = -1; i <= 1; ++i)
    {
        int floorIndex = currentFloor + i;

        float floorCenterZ =
            floorIndex * FLOOR_LENGTH
            + FLOOR_LENGTH * 0.5f;

        drawFloor(
            P,
            V,
            vec3(0.0f, floorCenterY(), floorCenterZ),
            vec3(FLOOR_WIDTH, FLOOR_THICKNESS, FLOOR_LENGTH)
        );
    }
}

void renderSideWalls(mat4 P, mat4 V) {

    int currentWallSegment = (int)floor(playerZ / SIDE_WALL_SEGMENT_LENGTH);

    // Dibujar segmentos alrededor del jugador
    for (int i = -2; i <= 2; ++i)
    {
        int segmentIndex =
            currentWallSegment + i;

        float segmentCenterZ =
            segmentIndex * SIDE_WALL_SEGMENT_LENGTH
            + SIDE_WALL_SEGMENT_LENGTH * 0.5f;

        // =====================================================
        // PARED IZQUIERDA
        // =====================================================

        drawWall(
            P,
            V,
            vec3(
                -xLimit - SIDE_WALL_OFFSET,
                0.0f,
                segmentCenterZ
            ),
            vec3(
                SIDE_WALL_THICKNESS,
                worldHeight(),
                SIDE_WALL_SEGMENT_LENGTH
            )
        );

        // =====================================================
        // PARED DERECHA
        // =====================================================

        drawWall(
            P,
            V,
            vec3(
                xLimit + SIDE_WALL_OFFSET,
                0.0f,
                segmentCenterZ
            ),
            vec3(
                SIDE_WALL_THICKNESS,
                worldHeight(),
                SIDE_WALL_SEGMENT_LENGTH
            )
        );
    }
}

vec3 getObstacleSegmentColor(
    int wallIndex,
    WallSegment segment
)
{
    if (
        gameOver &&
        wallIndex == hitWallIndex &&
        hitSegment == segment
    )
    {
        return HIT_COLOR;
    }

    return vec3(1.0f);
}

void drawObstacleSegment(
    mat4 P,
    mat4 V,
    vec3 position,
    vec3 scale,
    vec3 color
)
{
    dibujarObjetoConMaterial(
        cubo,
        P,
        V,
        position,
        scale,
        color,
        wallMaterial
    );
}

void renderObstacles(mat4 P, mat4 V)
{
    for (size_t i = 0; i < walls.size(); ++i)
    {
        const WallObstacle& w = walls[i];

        // =========================================
        // Dimensiones segmentos
        // =========================================

        float leftWidth = (w.gapX - holeHalfW) - (-worldHalfX);
        float rightWidth = worldHalfX - (w.gapX + holeHalfW);
        float bottomSegmentTop = w.gapY - holeHalfH;
        float topSegmentBottom = w.gapY + holeHalfH;
        float bottomHeight = bottomSegmentTop - obstacleBottomY();
        float topHeight = obstacleTopY() - topSegmentBottom;

        // =========================================
        // Colores
        // =========================================

        vec3 leftColor = getObstacleSegmentColor((int)i,SEG_LEFT);
        vec3 rightColor = getObstacleSegmentColor((int)i,SEG_RIGHT);
        vec3 bottomColor = getObstacleSegmentColor((int)i,SEG_BOTTOM);
        vec3 topColor = getObstacleSegmentColor((int)i,SEG_TOP);

        // =========================================
        // Segmento izquierdo
        // =========================================

        if (leftWidth > 0.0f)
        {
            drawObstacleSegment(
                P,
                V,
                vec3(
                    (-worldHalfX + (w.gapX - holeHalfW)) * 0.5f,
                    obstacleFullCenterY(),
                    w.z
                ),
                vec3(
                    leftWidth,
                    obstacleFullHeight(),
                    wallThickness
                ),
                leftColor
            );
        }

        // =========================================
        // Segmento derecho
        // =========================================

        if (rightWidth > 0.0f)
        {
            drawObstacleSegment(
                P,
                V,
                vec3(
                    ((w.gapX + holeHalfW) + worldHalfX) * 0.5f,
                    obstacleFullCenterY(),
                    w.z
                ),
                vec3(
                    rightWidth,
                    obstacleFullHeight(),
                    wallThickness
                ),
                rightColor
            );
        }

        // =========================================
        // Segmento inferior
        // =========================================

        if (bottomHeight > 0.0f)
        {
            drawObstacleSegment(
                P,
                V,
                vec3(
                    w.gapX,
                    (obstacleBottomY() + bottomSegmentTop) * 0.5f,
                    w.z
                ),
                vec3(
                    holeHalfW * 2.0f,
                    bottomHeight,
                    wallThickness
                ),
                bottomColor
            );
        }

        // =========================================
        // Segmento superior
        // =========================================

        if (topHeight > 0.0f)
        {
            drawObstacleSegment(
                P,
                V,
                vec3(
                    w.gapX,
                    (topSegmentBottom + obstacleTopY()) * 0.5f,
                    w.z
                ),
                vec3(
                    holeHalfW * 2.0f,
                    topHeight,
                    wallThickness
                ),
                topColor
            );
        }
    }
}

void renderSun(mat4 P, mat4 V)
{
    if (sunPosition.y < 0.0f)
        return;

    mat4 M(1.0f);

    M = translate(M, sunPosition);
    M = rotate(M, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
    M = scale(M, vec3(4.0f));

    drawSunSphere(P, V, M);
}
