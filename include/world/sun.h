#pragma once

#include <glm/glm.hpp>

extern glm::vec3 sunPosition;
extern glm::vec3 sunDirection;


const float SUNRISE_HOUR = 6.25f;
const float SUNSET_HOUR  = 18.75f;

void updateSun();
float getSunProgress();