#include "game/player.h"

#include "core/mathUtils.h"
#include "core/timing.h"
#include "game/gameState.h"
#include "graphics/renderer.h"
#include "graphics/lighting.h"
#include "graphics/meshes.h"
#include "graphics/shadowMapping.h"
#include "weather/fog.h"
#include "world/world.h"

// =========================================================
// Variables básicas del juego
// =========================================================
float playerX = 0.0f;
float playerY = 0.0f;
float playerZ = 0.0f;
float playerVelY = 0.0f;

float gravity = -3.5f;
float jumpImpulse = 1.6f;

float forwardSpeed = 2.6f;
float sideSpeed = 2.2f;
float xLimit = 2.5f;

float currentPitch = 0.0f;
float targetPitch  = 0.0f;

float currentRoll  = 0.0f;
float targetRoll   = 0.0f;

bool headlightsEnabled = false;

bool wasNightLastFrame = false;
bool wasFogLastFrame = false;


mat4 calcularMatrizJugador()
{
    mat4 T = glm::translate(vec3(
        playerX,
        playerY,
        playerZ
    ));
    mat4 R = calcularRotacionJugador();
    mat4 R_modelo = glm::rotate(mat4(1.0f), glm::radians(0.0f), vec3(0,1,0));
    mat4 S = glm::scale(vec3(0.90f));

    return T * R * R_modelo * S;
}

void updatePlayerRotation()
{
    // =========================================
    // Pitch objetivo
    // =========================================

    targetPitch = -playerVelY * 0.18f;

    targetPitch =
        glm::clamp(
            targetPitch,
            glm::radians(-25.0f),
            glm::radians(25.0f)
        );

    // =========================================
    // Roll objetivo
    // =========================================

    targetRoll = 0.0f;

    if (keyA && !keyD)
    {
        targetRoll = glm::radians(-20.0f);
    }
    else if (keyD && !keyA)
    {
        targetRoll = glm::radians(20.0f);
    }

    // =========================================
    // Interpolación
    // =========================================

    float rotationSpeed = 8.0f;

    currentPitch =
        glm::mix(
            currentPitch,
            targetPitch,
            deltaTime * rotationSpeed
        );

    currentRoll =
        glm::mix(
            currentRoll,
            targetRoll,
            deltaTime * rotationSpeed
        );
}

void dibujarJugador(mat4 P, mat4 V)
{
    glm::mat4 M = calcularMatrizJugador();

    glUseProgram(prog);

    glUniform3f(
        glGetUniformLocation(prog, "objectColor"),
        1.0f,
        1.0f,
        1.0f
    );

    glUniform1i(
        glGetUniformLocation(prog, "useTexture"),
        GL_TRUE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useBumpMap"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useVertexColor"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useNormalMap"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useRoughnessMap"),
        GL_FALSE
    );

    glUniform1f(
        glGetUniformLocation(prog, "normalMapStrength"),
        0.0f
    );

    glUniform1f(
        glGetUniformLocation(prog, "roughnessFactor"),
        0.45f
    );

    glUniform1i(
        glGetUniformLocation(prog, "useShadowMap"),
        GL_TRUE
    );

    transfer_mat4("lightSpaceMatrix", currentLightSpaceMatrix);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(prog, "shadowMap"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapLeft);
    glUniform1i(glGetUniformLocation(prog, "headlightShadowMapLeft"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapRight);
    glUniform1i(glGetUniformLocation(prog, "headlightShadowMapRight"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, headlightCookieTexture);
    glUniform1i(glGetUniformLocation(prog, "spotlightCookie"), 4);

    // Algunos modelos GLB tienen orientación de caras distinta.
    glDisable(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planeTexture);

    glBindVertexArray(avion.VAO);

    transfer_mat4("M", M);
    transfer_mat4("MVP", P * V * M);

    glDrawArrays(GL_TRIANGLES, 0, avion.Nv);

    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);

    // Dejamos el shader en modo normal para el resto de objetos
    glUniform1i(
        glGetUniformLocation(prog, "useVertexColor"),
        GL_FALSE
    );
}

void updatePlayerPhysics() {
    playerVelY += gravity * deltaTime;
    playerY += playerVelY * deltaTime;
}

void updatePlayerMovement() {
    if (keyA && !keyD) playerX += sideSpeed * deltaTime;
    if (keyD && !keyA) playerX -= sideSpeed * deltaTime;

    playerX = clampf(playerX, -xLimit, xLimit);

    playerZ += forwardSpeed * deltaTime;
}


void updatePlayerBounds() {
    if (playerY > worldHalfY || playerY < -worldHalfY)
    {
        enterGameOver();
    }
}

glm::mat4 calcularRotacionJugador()
{
    mat4 R_pitch =
        glm::rotate(
            mat4(1.0f),
            currentPitch,
            vec3(1,0,0)
        );

    mat4 R_roll =
        glm::rotate(
            mat4(1.0f),
            currentRoll,
            vec3(0,0,1)
        );

    return
        R_roll *
        R_pitch;
}
bool shouldAutoEnableHeadlights()
{
    return isNight() || fogDensity > 0.22f;
}

void updateLowVisibilityLastFrame() {
    wasNightLastFrame = isNight();
    wasFogLastFrame = fogDensity > 0.22f;
}

void updateHeadlights()
{
    bool currentlyNight = isNight();

    bool currentlyFoggy = fogDensity > 0.22f;

    if (currentlyNight &&!wasNightLastFrame)
    {
        headlightsEnabled = true;
    }

    if (currentlyFoggy && !wasFogLastFrame)
    {
        headlightsEnabled = true;
    }

    if (!currentlyNight && !currentlyFoggy && (wasNightLastFrame || wasFogLastFrame))
    {
        headlightsEnabled = false;
    }

    wasNightLastFrame = currentlyNight;
    wasFogLastFrame = currentlyFoggy;
}

void toggleHeadlights()
{
    headlightsEnabled = !headlightsEnabled;
}

