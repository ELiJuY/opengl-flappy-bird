#include "game/gameLoop.h"

#include "core/timing.h"
#include "game/collisionSystem.h"
#include "game/gameState.h"
#include "game/player.h"


void updateGame() {
    if (!isGamePlaying()) return;

    updateGameTime();

    updatePlayerPhysics();

    updatePlayerMovement();

    updateHeadlights();

    updatePlayerRotation();

    updatePlayerBounds();

    if (!isGamePlaying()) return;

    updateScore();

    recycleWalls();

    spawnWalls();

    updateCollisions();

}

void updateCollisions() {

    CollisionInfo hit = detectCollision();

    if (hit.hit)
    {
        enterGameOver();
        hitWallIndex = hit.wallIndex;
        hitSegment = hit.segment;
    }

}
