#pragma once

#include "world/walls.h"

enum GameState
{
    PLAYING,
    PAUSED,
    GAME_OVER
};

extern GameState currentGameState;
extern bool gameOver;

extern int hitWallIndex;

extern WallSegment hitSegment;

extern int score;

void reiniciarJuego();

void updateScore();

void setGameState(GameState newState);
void togglePause();
void enterGameOver();
bool isGamePlaying();
bool isGamePaused();
bool isGameOver();
