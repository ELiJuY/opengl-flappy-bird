#pragma once
#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

const float FLOOR_LENGTH = 24.8f;
const float FLOOR_WIDTH = 24.8f;
const float FLOOR_THICKNESS = 0.2f;

const float SIDE_WALL_OFFSET = 0.35f;
const float SIDE_WALL_THICKNESS = 0.35f;
const float SIDE_WALL_SEGMENT_LENGTH = 12.0f;

const glm::vec3 HIT_COLOR  = glm::vec3(0.90f, 0.10f, 0.10f);

void renderFloor(glm::mat4 P, glm::mat4 V);
void renderSideWalls(glm::mat4 P, glm::mat4 V);
void renderObstacles(glm::mat4 P, glm::mat4 V);
void renderSun(glm::mat4 P, glm::mat4 V);
