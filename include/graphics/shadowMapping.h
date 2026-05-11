#pragma once
#include <core/gpo_wrapper.h>

#include "glad/glad.h"
#include "glm/fwd.hpp"

extern GLuint depthMapFBO;
extern GLuint depthMap;

extern glm::mat4 currentLightSpaceMatrix;

void initShadowMap();
void renderShadowPass();
void renderHeadlightShadowPassLeft();
void renderHeadlightShadowPassRight();
glm::mat4 calcularLightSpaceMatrix();
glm::mat4 calcularHeadlightMatrix(glm::vec3 lightPos, glm::vec3 lightDir);
void renderFloorDepth(glm::mat4 lightSpaceMatrix);
void renderSideWallsDepth(glm::mat4 lightSpaceMatrix);
void renderObstaclesDepth(glm::mat4 lightSpaceMatrix);
void renderPlayerDepth(glm::mat4 lightSpaceMatrix);
void drawObjectDepth(const objeto& obj, const glm::mat4& lightSpaceMatrix, const vec3 t, const vec3 s);
void transferMat4ToProgram(GLuint program, const char* name, const glm::mat4& matrix);