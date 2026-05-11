#pragma once
#include "glm/vec3.hpp"


extern float turbulenceStart;
extern float turbulenceEnd;

extern glm::vec3 dawnSky;
extern glm::vec3 daySky;
extern glm::vec3 sunsetSky;
extern glm::vec3 nightSky;

extern glm::vec3 dawnLight;
extern glm::vec3 dayLight;
extern glm::vec3 sunsetLight;
extern glm::vec3 nightLight;

glm::vec3 calculateSkyLightColor(float dayTime);
void updateSky();
float getSunProgress();
