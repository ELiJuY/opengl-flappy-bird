#include "game/gameState.h"

#include <cstdio>
#include "game/player.h"
#include "world/walls.h"
#include "world/world.h"

GameState currentGameState = PLAYING;
bool gameOver = false;
int hitWallIndex = -1;
WallSegment hitSegment = SEG_NONE;
int score = 0;
bool headlightsWereEnabledBeforeGameOver = false;

void setGameState(GameState newState)
{
    if (newState == GAME_OVER && currentGameState != GAME_OVER)
    {
        headlightsWereEnabledBeforeGameOver = headlightsEnabled;
    }

    currentGameState = newState;
    gameOver = currentGameState == GAME_OVER;

    if (currentGameState == GAME_OVER)
    {
        headlightsEnabled = false;
    }
}

void togglePause()
{
    if (currentGameState == PLAYING)
    {
        setGameState(PAUSED);
    }
    else if (currentGameState == PAUSED)
    {
        setGameState(PLAYING);
    }
}

void enterGameOver()
{
    setGameState(GAME_OVER);
}

bool isGamePlaying()
{
    return currentGameState == PLAYING;
}

bool isGamePaused()
{
    return currentGameState == PAUSED;
}

bool isGameOver()
{
    return currentGameState == GAME_OVER;
}

void reiniciarJuego()
{
    playerX = 0.0f;
    playerY = 0.0f;
    playerZ = 0.0f;
    playerVelY = 0.0f;
    currentPitch = 0.0f;
    targetPitch = 0.0f;
    currentRoll = 0.0f;
    targetRoll = 0.0f;

    bool restoreHeadlights =
        isGameOver()
        ? headlightsWereEnabledBeforeGameOver
        : headlightsEnabled;

    setGameState(PLAYING);
    headlightsEnabled = restoreHeadlights;
    hitWallIndex = -1;
    hitSegment = SEG_NONE;

    score = 0;
    inicializarParedes();
}

void updateScore() {
    for (size_t i = 0; i < walls.size(); ++i)
    {
        if (!walls[i].scored && playerZ > walls[i].z + wallThickness)
        {
            walls[i].scored = true;
            score++;

            printf("Score: %d\n", score);
        }
    }
}
