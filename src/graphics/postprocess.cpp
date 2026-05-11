#include "graphics/postprocess.h"

#include <iostream>

#include "core/config.h"
#include "core/gpo_wrapper.h"

GLuint sceneFBO;
GLuint sceneColorTexture;
GLuint sceneDepthTexture;

static int sceneFramebufferWidth = 0;
static int sceneFramebufferHeight = 0;

void resizeSceneFramebuffer(int width, int height)
{
    if (
        width <= 0 ||
        height <= 0 ||
        sceneFBO == 0 ||
        sceneColorTexture == 0 ||
        sceneDepthTexture == 0
    )
    {
        return;
    }

    if (
        width == sceneFramebufferWidth &&
        height == sceneFramebufferHeight
    )
    {
        return;
    }

    sceneFramebufferWidth = width;
    sceneFramebufferHeight = height;

    glBindTexture(GL_TEXTURE_2D, sceneColorTexture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT24,
        width,
        height,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR scene framebuffer resize" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSceneFramebuffer()
{
    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // =========================================
    // Color texture
    // =========================================

    glGenTextures(1, &sceneColorTexture);
    glBindTexture(GL_TEXTURE_2D, sceneColorTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        sceneColorTexture,
        0
    );

    GLenum drawBuffers[] =
    {
        GL_COLOR_ATTACHMENT0
    };

    glDrawBuffers(1, drawBuffers);

    // =========================================
    // Depth texture
    // =========================================

    glGenTextures(1, &sceneDepthTexture);
    glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        sceneDepthTexture,
        0
    );

    resizeSceneFramebuffer(ANCHO, ALTO);
}
