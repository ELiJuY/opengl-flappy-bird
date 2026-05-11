#pragma once
#include "glm/fwd.hpp"
#include "glm/vec3.hpp"


extern glm::vec3 pos_obs;
extern glm::vec3 target;
extern glm::vec3 up;

extern float fov;
extern float aspect;

extern float nearPlane;
extern float farPlane;

void updateCamera(glm::mat4& P, glm::mat4& V);
