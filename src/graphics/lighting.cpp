#include "graphics/lighting.h"

#include "core/timing.h"
#include "game/player.h"
#include "glm/gtc/type_ptr.inl"
#include "graphics/camera.h"
#include "graphics/renderer.h"
#include "graphics/sky.h"
#include "world/sun.h"

glm::vec3 spotlightLeftPos(0.0f);
glm::vec3 spotlightRightPos(0.0f);
glm::vec3 spotlightDirection(0.0f);
float headlightsFactor;
glm::vec3 spotLightColor(1.0f, 0.95f, 0.82f);
float daylightFactor = 0.0f;


void updateSunLighting()
{
    glUniform3f(
        glGetUniformLocation(prog, "lightDir"),
        sunDirection.x,
        sunDirection.y,
        sunDirection.z
    );

    float progress = getSunProgress();
    daylightFactor = 0.0f;

    if (progress >= 0.0f)
    {
        daylightFactor = sin(progress * glm::pi<float>());
    }

    glUniform1f(
        glGetUniformLocation(prog, "daylightFactor"),
        daylightFactor
    );

    vec3 dynamicLightColor = calculateSkyLightColor(simulatedHour);

    glUniform3f(
        glGetUniformLocation(prog, "lightColor"),
        dynamicLightColor.r,
        dynamicLightColor.g,
        dynamicLightColor.b
    );
}

void updateFillLight()
{
    vec3 fillLightPos(
        playerX,
        playerY + 2.0f,
        playerZ - 3.0f
    );

    vec3 fillLightColor(
        0.25f,
        0.30f,
        0.40f
    );

    glUniform3f(
        glGetUniformLocation(prog, "fillLightPos"),
        fillLightPos.x,
        fillLightPos.y,
        fillLightPos.z
    );

    glUniform3f(
        glGetUniformLocation(prog, "fillLightColor"),
        fillLightColor.x,
        fillLightColor.y,
        fillLightColor.z
    );
}

void updateHeadlightMatrices(
    vec3 leftPos,
    vec3 rightPos,
    vec3 dir,
    vec3 up
)
{
    mat4 projection =
        glm::perspective(
            glm::radians(40.0f),
            1.0f,
            0.1f,
            35.0f
        );

    mat4 leftView =
        glm::lookAt(
            leftPos,
            leftPos + dir,
            vec3(0,1,0)
        );

    mat4 rightView =
        glm::lookAt(
            rightPos,
            rightPos + dir,
            up
        );

    headlightMatrixLeft = projection * leftView;
    headlightMatrixRight = projection * rightView;
}

void updateSpotlights()
{
    headlightsFactor = headlightsEnabled ? 1.0f : 0.0f;
    mat4 R = calcularRotacionJugador();
    vec3 planeForward =
            normalize(
                vec3(
                    R * vec4(0, 0, 1, 0)
                )
            );

    spotlightDirection = planeForward;

    vec3 planeRight =
        normalize(
            vec3(
                R *
                vec4(1,0,0,0)
            )
        );

    vec3 basePos(
        playerX,
        playerY,
        playerZ
    );

    spotlightLeftPos =
            basePos +
            planeForward * 0.8f -
            planeRight * 0.20f;

    spotlightRightPos =
            basePos +
            planeForward * 0.8f +
            planeRight * 0.20f;

    vec3 planeUp = normalize(vec3(R * vec4(0, 1, 0, 0)));

    updateHeadlightMatrices(
        spotlightLeftPos,
        spotlightRightPos,
        spotlightDirection,
        planeUp
    );

    float nightFactor =
        glm::clamp(
            1.0f - daylightFactor,
            0.0f,
            1.0f
        );

    float headlightColorIntensity =
        glm::mix(
            1.15f,
            2.4f,
            glm::pow(nightFactor, 1.1f)
        );

    spotLightColor =
        vec3(1.0f, 0.95f, 0.82f) *
        headlightColorIntensity;
    glUniform3f(
        glGetUniformLocation(prog, "spotlightLeftPos"),
        spotlightLeftPos.x,
        spotlightLeftPos.y,
        spotlightLeftPos.z
    );

    glUniform3f(
        glGetUniformLocation(prog, "spotlightRightPos"),
        spotlightRightPos.x,
        spotlightRightPos.y,
        spotlightRightPos.z
    );

    glUniform3f(
        glGetUniformLocation(prog, "spotlightDirection"),
        spotlightDirection.x,
        spotlightDirection.y,
        spotlightDirection.z
    );

    glUniform3f(
        glGetUniformLocation(prog, "spotLightColor"),
        spotLightColor.x,
        spotLightColor.y,
        spotLightColor.z
    );

    glUniform1f(
        glGetUniformLocation(prog, "spotlightCutoff"),
        glm::cos(glm::radians(10.0f))
    );

    glUniform1f(
        glGetUniformLocation(prog, "spotlightOuterCutoff"),
        glm::cos(glm::radians(18.0f))
    );

    glUniform1f(
        glGetUniformLocation(prog, "headlightsFactor"),
        headlightsFactor
    );
}

void updateViewPosition()
{
    glUniform3f(
        glGetUniformLocation(prog, "viewPos"),
        pos_obs.x,
        pos_obs.y,
        pos_obs.z
    );
}

void updateHeadlightUniforms()
{
    glUniformMatrix4fv(
        glGetUniformLocation(
            prog,
            "headlightMatrixLeft"
        ),
        1,
        GL_FALSE,
        glm::value_ptr(headlightMatrixLeft)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(
            prog,
            "headlightMatrixRight"
        ),
        1,
        GL_FALSE,
        glm::value_ptr(headlightMatrixRight)
    );
}

void updateLighting() {
    glUseProgram(prog);

    updateSunLighting();
    updateFillLight();
    updateSpotlights();
    updateViewPosition();
    updateHeadlightUniforms();
}
