#include "core/gpo_wrapper.h"
#include "core/frame.h"
#include "core/timing.h"
#include "game/gameState.h"

void beginFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float wallTime = (float)glfwGetTime();

    deltaTime = wallTime - lastTime;

    lastTime = wallTime;

    if (deltaTime > 0.05f)
        deltaTime = 0.05f;

    if (!isGamePlaying())
    {
        deltaTime = 0.0f;
        return;
    }

    currentTime += deltaTime;
}
