#pragma once
#include "glad/glad.h"
#include "glm/fwd.hpp"

extern GLuint headlightShadowFBOLeft;
extern GLuint headlightShadowFBORight;

extern GLuint headlightShadowMapLeft;
extern GLuint headlightShadowMapRight;

extern glm::mat4 headlightMatrixLeft;
extern glm::mat4 headlightMatrixRight;

extern glm::vec3 spotlightLeftPos;
extern glm::vec3 spotlightRightPos;
extern glm::vec3 spotlightDirection;
extern float headlightsFactor;
extern glm::vec3 spotLightColor;
extern float daylightFactor;

void updateLighting();
void updateSunLighting();
void updateFillLight();
void updateSpotlights();
void updateViewPosition();
void initHeadlightShadowMaps();
void updateHeadlightUniforms();