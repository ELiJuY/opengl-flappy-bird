#pragma once
#include "glad/glad.h"

extern GLuint sceneFBO;
extern GLuint sceneColorTexture;
extern GLuint sceneDepthTexture;

extern const char* screen_vertex_prog;
extern const char* screen_fragment_prog;

void initSceneFramebuffer();
void resizeSceneFramebuffer(int width, int height);
