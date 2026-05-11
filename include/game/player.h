#pragma once
#include "glm/fwd.hpp"
#include <glm/glm.hpp>

extern float playerX;
extern float playerY;
extern float playerZ;

extern float currentPitch;
extern float targetPitch;

extern float currentRoll;
extern float targetRoll;

extern float playerVelY;

extern float gravity;
extern float jumpImpulse;

extern float forwardSpeed;
extern float sideSpeed;

extern float xLimit;

extern bool headlightsEnabled;
extern bool headlightsManualOverride;
extern bool wasFogLastFrame;


void dibujarJugador(
    glm::mat4 P,
    glm::mat4 V
);

void updatePlayerPhysics();
void updatePlayerMovement();
void updatePlayerBounds();
void updatePlayerRotation();

glm::mat4 calcularRotacionJugador();
glm::mat4 calcularMatrizJugador();

void updateHeadlights();
void toggleHeadlights();