#include "weather/fog.h"

#include <iostream>

#include <glm/glm.hpp>

#include "core/timing.h"
#include "graphics/renderer.h"
#include "graphics/shaders.h"
#include "core/mathUtils.h"

enum FogState
{
    FOG_CLEAR,
    FOG_FADE_IN,
    FOG_HOLD,
    FOG_FADE_OUT
};

static FogState fogState = FOG_CLEAR;

float fogDensity = 0.0f;
static float fogMaxDensity = 0.0f;

static float fogFadeInDuration = 0.0f;
static float fogHoldDuration = 0.0f;
static float fogFadeOutDuration = 0.0f;

static float fogPhaseTimer = 0.0f;
static float fogPhaseStartTime = 0.0f;

static float nextFogTime = 0.0f;


static void configureFogEvent(float waitMin, float waitMax);
static void scheduleNextFog();

static void updateFogClear();
static void updateFogFadeIn();
static void updateFogHold();
static void updateFogFadeOut();

void initFog()
{
    scheduleNextFog();
}


static void configureFogEvent(
    float waitMin,
    float waitMax
)
{
    nextFogTime = currentTime + randomFloat(waitMin, waitMax);
    fogMaxDensity = randomFloat(0.30f, 0.50f);
    fogFadeInDuration = randomFloat(8.0f, 12.0f);
    fogHoldDuration = randomFloat(2.0f, 4.0f);
    fogFadeOutDuration = randomFloat(8.0f, 12.0f);

    std::cout
        << "[FOG] Next fog in "
        << (nextFogTime - currentTime)
        << "s | duration: "
        << (
            fogFadeInDuration +
            fogHoldDuration +
            fogFadeOutDuration
        )
        << "s | max density: "
        << fogMaxDensity
        << std::endl;
}

static void scheduleNextFog()
{
    configureFogEvent(25.0f, 40.0f);
}

void updateFog()
{
    switch (fogState)
    {
        case FOG_CLEAR:
            updateFogClear();
            break;

        case FOG_FADE_IN:
            updateFogFadeIn();
            break;

        case FOG_HOLD:
            updateFogHold();
            break;

        case FOG_FADE_OUT:
            updateFogFadeOut();
            break;
    }
}

static void updateFogClear()
{
    fogDensity = 0.0f;

    if (currentTime >= nextFogTime)
    {
        fogState = FOG_FADE_IN;

        fogPhaseTimer = 0.0f;
        fogPhaseStartTime = currentTime;

        std::cout
            << "[FOG] FADE IN"
            << std::endl;
    }
}

static void updateFogFadeIn()
{
    fogPhaseTimer = currentTime - fogPhaseStartTime;
    float t = fogPhaseTimer / fogFadeInDuration;
    t = glm::clamp(t, 0.0f, 1.0f);
    float smoothT = t * t * (3.0f - 2.0f * t);
    fogDensity = fogMaxDensity * smoothT;

    if (t >= 1.0f)
    {
        fogState = FOG_HOLD;

        fogPhaseTimer = 0.0f;
        fogPhaseStartTime = currentTime;

        fogDensity = fogMaxDensity;

        std::cout
            << "[FOG] HOLD"
            << std::endl;
    }
}

static void updateFogHold()
{
    fogPhaseTimer = currentTime - fogPhaseStartTime;

    fogDensity = fogMaxDensity;

    if (fogPhaseTimer >= fogHoldDuration)
    {
        fogState = FOG_FADE_OUT;

        fogPhaseTimer = 0.0f;
        fogPhaseStartTime = currentTime;

        std::cout
            << "[FOG] FADE OUT"
            << std::endl;
    }
}

static void updateFogFadeOut()
{
    fogPhaseTimer = currentTime - fogPhaseStartTime;
    float t = fogPhaseTimer / fogFadeOutDuration;
    t = glm::clamp(t, 0.0f, 1.0f);
    float smoothT = t * t * (3.0f - 2.0f * t);

    fogDensity = fogMaxDensity * (1.0f - smoothT);

    if (t >= 1.0f)
    {
        fogDensity = 0.0f;

        fogState = FOG_CLEAR;

        fogPhaseTimer = 0.0f;
        fogPhaseStartTime = currentTime;

        scheduleNextFog();

        std::cout
            << "[FOG] END"
            << std::endl;
    }
}

void transferFogUniforms()
{
    bool fogVisible = fogDensity > 0.01f;

    glUniform1i(
        glGetUniformLocation(prog, "useFog"),
        fogVisible ? GL_TRUE : GL_FALSE
    );

    glUniform1f(
        glGetUniformLocation(prog, "fogDensity"),
        fogDensity
    );

    glUniform1f(
        glGetUniformLocation(prog, "fogStart"),
        0.0f
    );

    glUniform1f(
        glGetUniformLocation(prog, "fogEnd"),
        5.0f
    );

    glUniform3f(
        glGetUniformLocation(prog, "fogColor"),
        0.78f,
        0.82f,
        0.84f
    );
}