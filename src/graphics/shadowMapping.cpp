#include <graphics/shadowMapping.h>

#include <iostream>
#include <core/gpo_wrapper.h>
#include "core/config.h"
#include "game/player.h"
#include "glm/gtc/type_ptr.hpp"
#include "graphics/lighting.h"
#include "graphics/meshes.h"
#include "graphics/renderer.h"
#include "graphics/worldRenderer.h"
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
}

GLuint depthMapFBO = 0;
GLuint depthMap = 0;

glm::mat4 currentLightSpaceMatrix = glm::mat4(1.0f);
glm::mat4 headlightMatrixLeft(1.0f);
glm::mat4 headlightMatrixRight(1.0f);

GLuint headlightShadowFBOLeft;
GLuint headlightShadowFBORight;

GLuint headlightShadowMapLeft;
GLuint headlightShadowMapRight;


void initShadowMap()
{
    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        SHADOW_WIDTH,
        SHADOW_HEIGHT,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glTexParameterfv(
        GL_TEXTURE_2D,
        GL_TEXTURE_BORDER_COLOR,
        borderColor
    );

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        depthMap,
        0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: shadow framebuffer no esta completo." << std::endl;
    }
    else
    {
        std::cout << "Shadow framebuffer creado correctamente." << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initHeadlightShadowMaps()
{
    const unsigned int SHADOW_SIZE = 1024;

    // =========================================
    // LEFT
    // =========================================

    glGenFramebuffers(1, &headlightShadowFBOLeft);

    glGenTextures(1, &headlightShadowMapLeft);

    glBindTexture(
        GL_TEXTURE_2D,
        headlightShadowMapLeft
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        SHADOW_SIZE,
        SHADOW_SIZE,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        NULL
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_BORDER
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_BORDER
    );

    float borderColor[] =
    {
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };

    glTexParameterfv(
        GL_TEXTURE_2D,
        GL_TEXTURE_BORDER_COLOR,
        borderColor
    );

    glBindFramebuffer(
        GL_FRAMEBUFFER,
        headlightShadowFBOLeft
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        headlightShadowMapLeft,
        0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: headlight shadow FBO izq." << std::endl;
    }
    else
    {
        std::cout << "Headlight shadow FBO izq creado correctamente." << std::endl;
    }

    // =========================================
    // RIGHT
    // =========================================

    glGenFramebuffers(1, &headlightShadowFBORight);

    glGenTextures(1, &headlightShadowMapRight);

    glBindTexture(
        GL_TEXTURE_2D,
        headlightShadowMapRight
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        SHADOW_SIZE,
        SHADOW_SIZE,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        NULL
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_BORDER
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_BORDER
    );

    glTexParameterfv(
        GL_TEXTURE_2D,
        GL_TEXTURE_BORDER_COLOR,
        borderColor
    );

    glBindFramebuffer(
        GL_FRAMEBUFFER,
        headlightShadowFBORight
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        headlightShadowMapRight,
        0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: headlight shadow FBO der." << std::endl;
    }
    else
    {
        std::cout << "Headlight shadow FBO der creado correctamente." << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderShadowPass()
{
    currentLightSpaceMatrix = calcularLightSpaceMatrix();

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(shadowProg);

    // Ayuda a evitar algunos artefactos de shadow acne
    glCullFace(GL_FRONT);

    renderFloorDepth(currentLightSpaceMatrix);
    renderSideWallsDepth(currentLightSpaceMatrix);
    renderObstaclesDepth(currentLightSpaceMatrix);
    renderPlayerDepth(currentLightSpaceMatrix);

    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, ANCHO, ALTO);
}

void renderHeadlightShadowPassLeft()
{
    headlightMatrixLeft =
        calcularHeadlightMatrix(
            spotlightLeftPos,
            spotlightDirection
        );

    glViewport(0, 0, 1024, 1024);

    glBindFramebuffer(
        GL_FRAMEBUFFER,
        headlightShadowFBOLeft
    );

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(shadowProg);

    glDisable(GL_CULL_FACE);

    renderFloorDepth(headlightMatrixLeft);
    renderSideWallsDepth(headlightMatrixLeft);
    renderObstaclesDepth(headlightMatrixLeft);
    renderPlayerDepth(headlightMatrixLeft);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, ANCHO, ALTO);
}

void renderHeadlightShadowPassRight()
{
    headlightMatrixRight =
        calcularHeadlightMatrix(
            spotlightRightPos,
            spotlightDirection
        );

    glViewport(0, 0, 1024, 1024);

    glBindFramebuffer(
        GL_FRAMEBUFFER,
        headlightShadowFBORight
    );

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(shadowProg);

    glDisable(GL_CULL_FACE);

    renderFloorDepth(headlightMatrixRight);
    renderSideWallsDepth(headlightMatrixRight);
    renderObstaclesDepth(headlightMatrixRight);
    renderPlayerDepth(headlightMatrixRight);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, ANCHO, ALTO);
}

glm::mat4 calcularLightSpaceMatrix()
{
    glm::vec3 sceneCenter(
        playerX,
        0.0f,
        playerZ + 10.0f
    );

    glm::vec3 lightDir = sunDirection;

    if (glm::length(lightDir) < 0.001f)
    {
        lightDir = glm::vec3(0.0f, -1.0f, 0.0f);
    }
    else
    {
        lightDir = glm::normalize(lightDir);
    }

    glm::vec3 lightPos =
        sceneCenter -
        lightDir * 60.0f;

    glm::vec3 up(0.0f, 1.0f, 0.0f);

    if (glm::abs(glm::dot(lightDir, up)) > 0.92f)
    {
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::mat4 lightProjection = glm::ortho(
        -18.0f,
         18.0f,
        -14.0f,
         14.0f,
          1.0f,
        140.0f
    );

    glm::mat4 lightView = glm::lookAt(
        lightPos,
        sceneCenter,
        up
    );

    return lightProjection * lightView;
}

glm::mat4 calcularHeadlightMatrix(
    glm::vec3 lightPos,
    glm::vec3 lightDir
)
{
    glm::mat4 projection =
        glm::perspective(
            glm::radians(40.0f),
            1.0f,
            0.1f,
            35.0f
        );

    glm::mat4 view =
        glm::lookAt(
            lightPos,
            lightPos + lightDir,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

    return projection * view;
}

void renderFloorDepth(glm::mat4 lightSpaceMatrix)
{
    int currentFloor =
        (int)floor(playerZ / FLOOR_LENGTH);

    for (int i = -1; i <= 1; ++i)
    {
        int floorIndex = currentFloor + i;

        float floorCenterZ =
            floorIndex * FLOOR_LENGTH
            + FLOOR_LENGTH * 0.5f;

        drawObjectDepth(
            suelo,
            lightSpaceMatrix,
            vec3(0.0f, floorCenterY(), floorCenterZ),
            vec3(FLOOR_WIDTH, FLOOR_THICKNESS, FLOOR_LENGTH)
        );
    }
}

void renderSideWallsDepth(glm::mat4 lightSpaceMatrix)
{
    int currentWallSegment =
        (int)floor(playerZ / SIDE_WALL_SEGMENT_LENGTH);

    for (int i = -2; i <= 2; ++i)
    {
        int segmentIndex =
            currentWallSegment + i;

        float segmentCenterZ =
            segmentIndex * SIDE_WALL_SEGMENT_LENGTH
            + SIDE_WALL_SEGMENT_LENGTH * 0.5f;

        drawObjectDepth(
            pared,
            lightSpaceMatrix,
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

        drawObjectDepth(
            pared,
            lightSpaceMatrix,
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

void renderObstaclesDepth(glm::mat4 lightSpaceMatrix)
{
    for (size_t i = 0; i < walls.size(); ++i)
    {
        const WallObstacle& w = walls[i];

        float leftWidth =
            (w.gapX - holeHalfW) - (-worldHalfX);

        float rightWidth =
            worldHalfX - (w.gapX + holeHalfW);

        float bottomHeight =
            (w.gapY - holeHalfH) - (-worldHalfY);

        float topHeight =
            worldHalfY - (w.gapY + holeHalfH);

        if (leftWidth > 0.0f)
        {
            drawObjectDepth(
                cubo,
                lightSpaceMatrix,
                vec3(
                    (-worldHalfX + (w.gapX - holeHalfW)) * 0.5f,
                    0.0f,
                    w.z
                ),
                vec3(
                    leftWidth,
                    worldHeight(),
                    wallThickness
                )
            );
        }

        if (rightWidth > 0.0f)
        {
            drawObjectDepth(
                cubo,
                lightSpaceMatrix,
                vec3(
                    ((w.gapX + holeHalfW) + worldHalfX) * 0.5f,
                    0.0f,
                    w.z
                ),
                vec3(
                    rightWidth,
                    worldHeight(),
                    wallThickness
                )
            );
        }

        if (bottomHeight > 0.0f)
        {
            drawObjectDepth(
                cubo,
                lightSpaceMatrix,
                vec3(
                    w.gapX,
                    (-worldHalfY + (w.gapY - holeHalfH)) * 0.5f,
                    w.z
                ),
                vec3(
                    holeHalfW * 2.0f,
                    bottomHeight,
                    wallThickness
                )
            );
        }

        if (topHeight > 0.0f)
        {
            drawObjectDepth(
                cubo,
                lightSpaceMatrix,
                vec3(
                    w.gapX,
                    ((w.gapY + holeHalfH) + worldHalfY) * 0.5f,
                    w.z
                ),
                vec3(
                    holeHalfW * 2.0f,
                    topHeight,
                    wallThickness
                )
            );
        }
    }
}

void renderPlayerDepth(glm::mat4 lightSpaceMatrix)
{
    mat4 M = calcularMatrizJugador();

    glUseProgram(shadowProg);

    transferMat4ToProgram(shadowProg, "M", M);
    transferMat4ToProgram(shadowProg, "lightSpaceMatrix", lightSpaceMatrix);

    // El modelo GLB puede tener caras con orientación irregular.
    // Para asegurar que proyecta sombra completo, desactivamos culling solo aquí.
    glDisable(GL_CULL_FACE);

    glBindVertexArray(avion.VAO);
    glDrawArrays(GL_TRIANGLES, 0, avion.Nv);

    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
}

void drawObjectDepth(
    const objeto& obj,
    const glm::mat4& lightSpaceMatrix,
    const vec3 t,
    const vec3 s
)
{
    glm::mat4 M = glm::translate(t) * glm::scale(s);
    transferMat4ToProgram(shadowProg, "M", M);
    transferMat4ToProgram(shadowProg, "lightSpaceMatrix", lightSpaceMatrix);

    glBindVertexArray(obj.VAO);
    glDrawArrays(GL_TRIANGLES, 0, obj.Nv);
    glBindVertexArray(0);
}

void transferMat4ToProgram(GLuint program, const char* name, const glm::mat4& matrix)
{
    GLint location = glGetUniformLocation(program, name);

    if (location >= 0)
    {
        glUniformMatrix4fv(
            location,
            1,
            GL_FALSE,
            glm::value_ptr(matrix)
        );
    }
}

