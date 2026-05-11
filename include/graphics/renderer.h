#pragma once

#include <core/gpo_wrapper.h>
#include "graphics/meshes.h"

extern GLFWwindow* window;

extern GLuint prog;
extern GLuint sunProg;
extern GLuint shadowProg;
extern GLuint screenProg;

extern GLuint floorTexture;
extern GLuint wallTexture;
extern GLuint sunTexture;
extern GLuint planeTexture;
extern GLuint headlightCookieTexture;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

struct RenderMaterial
{
    GLuint albedoTexture = 0;
    GLuint normalTexture = 0;
    GLuint roughnessTexture = 0;

    float normalStrength = 1.0f;
    float roughnessFactor = 0.65f;
};

extern RenderMaterial floorMaterial;
extern RenderMaterial wallMaterial;

void initScene();

void dibujarObjeto(
    objeto obj,
    mat4 P,
    mat4 V,
    vec3 t,
    vec3 s,
    vec3 color,
    GLuint texture = 0,
    GLenum textureUnit = GL_TEXTURE0,
    bool useBumpMap = false,
    float bumpStrength = 0.0f
);

void dibujarObjetoConMaterial(
    objeto obj,
    mat4 P,
    mat4 V,
    vec3 t,
    vec3 s,
    vec3 color,
    const RenderMaterial& material
);

void drawFloor(
    mat4 P,
    mat4 V,
    vec3 t,
    vec3 s
);

void drawWall(
    mat4 P,
    mat4 V,
    vec3 t,
    vec3 s
);

void renderScene();

void drawSunSphere(
    glm::mat4 P,
    glm::mat4 V,
    glm::mat4 M
);

