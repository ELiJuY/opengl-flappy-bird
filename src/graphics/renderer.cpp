#include "graphics/renderer.h"

#include <fstream>
#include <iostream>
#include "core/config.h"
#include "core/debug.h"
#include "core/frame.h"
#include "core/timing.h"
#include "game/gameLoop.h"
#include "game/gameState.h"
#include "game/player.h"
#include "glm/gtc/type_ptr.hpp"
#include "graphics/camera.h"
#include "graphics/lighting.h"
#include "graphics/meshes.h"
#include "graphics/postprocess.h"
#include "graphics/shaders.h"
#include "graphics/shadowMapping.h"
#include "graphics/sky.h"
#include "graphics/sunShaders.h"
#include "graphics/worldRenderer.h"
#include "weather/fog.h"
#include "world/world.h"
#include "world/sun.h"
#include <stb_image.h>

GLFWwindow* window = nullptr;

GLuint prog = 0;
GLuint sunProg = 0;
GLuint shadowProg = 0;
GLuint screenProg = 0;
GLuint dangerCeilingProg = 0;

GLuint floorTexture = 0;
GLuint wallTexture = 0;
GLuint sunTexture = 0;
GLuint planeTexture = 0;
GLuint headlightCookieTexture = 0;

RenderMaterial floorMaterial;
RenderMaterial wallMaterial;

GLuint screenQuadVAO = 0;
GLuint screenQuadVBO = 0;

bool existeArchivo(const char* ruta)
{
    std::ifstream f(ruta, std::ios::binary);
    return f.good();
}

GLuint cargarTexturaSegura(
    const char* ruta,
    GLenum textureUnit,
    const char* etiqueta
)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(ruta, &width, &height, &channels, 4);

    if (data == nullptr)
    {
        std::cout
            << "No se pudo cargar textura "
            << etiqueta
            << ": "
            << ruta
            << std::endl;

        return 0;
    }

    GLuint textureID = 0;
    glActiveTexture(textureUnit);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    std::cout
        << "Textura "
        << etiqueta
        << " cargada: "
        << ruta
        << " ("
        << width
        << "x"
        << height
        << ")"
        << std::endl;

    return textureID;
}

void renderLightDebug(mat4 P, mat4 V)
{
    for (int i = 0; i < 20; i++)
    {
        vec3 p =
            sunPosition +
            sunDirection *
            ((float)i * 2.0f);

        mat4 M(1.0f);

        M = translate(M, p);

        M = scale(M, vec3(0.25f));

        drawSunSphere(P, V, M);
    }
}

#define GLSL_RENDERER(src) "#version 330 core\n" #src

const char* depth_vertex_prog = GLSL_RENDERER(
layout(location = 0) in vec3 pos;

uniform mat4 lightSpaceMatrix = mat4(1.0f);
uniform mat4 M = mat4(1.0f);

void main()
{
    gl_Position = lightSpaceMatrix * M * vec4(pos, 1.0f);
}
);

const char* depth_fragment_prog = GLSL_RENDERER(
void main()
{
    // Solo se escribe profundidad.
}
);

const char* danger_ceiling_vertex_prog = GLSL_RENDERER(
layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

uniform mat4 MVP;
uniform mat4 M;

out vec3 FragPos;
out vec2 TexCoord;

void main()
{
    vec4 worldPos = M * vec4(pos, 1.0);

    FragPos = worldPos.xyz;
    TexCoord = uv;
    gl_Position = MVP * vec4(pos, 1.0);
}
);

const char* danger_ceiling_fragment_prog = GLSL_RENDERER(
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 cameraPosition;
uniform float dangerAlpha;
uniform float currentTime;
uniform float fogDensity;

out vec4 outputColor;

float hash(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float valueNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(hash(i), hash(i + vec2(1.0, 0.0)), u.x),
        mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x),
        u.y
    );
}

void main()
{
    vec2 panelUV = clamp(TexCoord / 4.0, 0.0, 1.0);
    vec3 viewDir = normalize(cameraPosition - FragPos);
    vec3 planeNormal = vec3(0.0, 1.0, 0.0);

    float edgeFade =
        smoothstep(0.00, 0.12, panelUV.x) *
        (1.0 - smoothstep(0.88, 1.00, panelUV.x)) *
        smoothstep(0.00, 0.10, panelUV.y) *
        (1.0 - smoothstep(0.90, 1.00, panelUV.y));

    float fresnel =
        pow(1.0 - clamp(abs(dot(viewDir, planeNormal)), 0.0, 1.0), 2.4);

    float scan =
        smoothstep(
            0.60,
            1.0,
            sin(FragPos.z * 8.5 - currentTime * 5.8) * 0.5 + 0.5
        );

    float grid =
        smoothstep(
            0.86,
            1.0,
            sin(FragPos.x * 7.5) * 0.5 + 0.5
        );

    float noise =
        valueNoise(FragPos.xz * 1.6 + vec2(currentTime * 0.28, -currentTime * 0.18));

    float pulse =
        0.76 + 0.24 * sin(currentTime * 4.0);

    float fogBoost =
        mix(0.88, 1.22, clamp(fogDensity / 0.45, 0.0, 1.0));

    float structure =
        0.16 +
        fresnel * 0.42 +
        scan * 0.22 +
        grid * 0.12 +
        noise * 0.18;

    float alpha =
        dangerAlpha *
        edgeFade *
        structure *
        pulse *
        fogBoost;

    vec3 redCore = vec3(0.72, 0.04, 0.025);
    vec3 redGlow = vec3(1.35, 0.08, 0.035);
    vec3 color = mix(redCore, redGlow, clamp(fresnel + scan * 0.35, 0.0, 1.0));

    outputColor = vec4(color, clamp(alpha, 0.0, 0.58));
}
);



void dibujarObjeto(
    objeto obj,
    mat4 P,
    mat4 V,
    vec3 t,
    vec3 s,
    vec3 color,
    GLuint texture,
    GLenum textureUnit,
    bool useBumpMap,
    float bumpStrength
)
{
    glUseProgram(prog);

    mat4 M =
        glm::translate(t) *
        glm::scale(s);

    glUniform3f(
        glGetUniformLocation(prog, "objectColor"),
        color.x,
        color.y,
        color.z
    );

    bool tieneTextura = texture != 0;

    glUniform1i(
        glGetUniformLocation(prog, "useTexture"),
        tieneTextura ? GL_TRUE : GL_FALSE
    );

    // Por defecto, los objetos normales no usan color por vértice.
    // El avión lo activa/desactiva dentro de dibujarJugador().
    glUniform1i(
        glGetUniformLocation(prog, "useVertexColor"),
        GL_FALSE
    );

    // Activamos bump mapping solo si el objeto tiene textura.
    glUniform1i(
        glGetUniformLocation(prog, "useBumpMap"),
        (tieneTextura && useBumpMap) ? GL_TRUE : GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useNormalMap"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useRoughnessMap"),
        GL_FALSE
    );

    glUniform1f(
        glGetUniformLocation(prog, "normalMapStrength"),
        0.0f
    );

    glUniform1f(
        glGetUniformLocation(prog, "roughnessFactor"),
        0.55f
    );

    glUniform1f(
        glGetUniformLocation(prog, "bumpStrength"),
        bumpStrength
    );

    glUniform1i(
    glGetUniformLocation(prog, "useShadowMap"),
    GL_TRUE
);

    transfer_mat4("lightSpaceMatrix", currentLightSpaceMatrix);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(prog, "shadowMap"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapLeft);
    glUniform1i(glGetUniformLocation(prog, "headlightShadowMapLeft"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapRight);
    glUniform1i(glGetUniformLocation(prog, "headlightShadowMapRight"), 3);



    if (tieneTextura)
    {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    glBindVertexArray(obj.VAO);

    transfer_mat4("M", M);
    transfer_mat4("MVP", P * V * M);

    glDrawArrays(GL_TRIANGLES, 0, obj.Nv);

    glBindVertexArray(0);
}

void dibujarObjetoConMaterial(
    objeto obj,
    mat4 P,
    mat4 V,
    vec3 t,
    vec3 s,
    vec3 color,
    const RenderMaterial& material
)
{
    glUseProgram(prog);

    mat4 M =
        glm::translate(t) *
        glm::scale(s);

    glUniform3f(
        glGetUniformLocation(prog, "objectColor"),
        color.x,
        color.y,
        color.z
    );

    bool hasAlbedo = material.albedoTexture != 0;
    bool hasNormal = material.normalTexture != 0;
    bool hasRoughness = material.roughnessTexture != 0;

    glUniform1i(
        glGetUniformLocation(prog, "useTexture"),
        hasAlbedo ? GL_TRUE : GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useVertexColor"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useBumpMap"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useNormalMap"),
        hasNormal ? GL_TRUE : GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useRoughnessMap"),
        hasRoughness ? GL_TRUE : GL_FALSE
    );

    glUniform1f(
        glGetUniformLocation(prog, "normalMapStrength"),
        material.normalStrength
    );

    glUniform1f(
        glGetUniformLocation(prog, "roughnessFactor"),
        material.roughnessFactor
    );

    glUniform1i(
        glGetUniformLocation(prog, "useShadowMap"),
        GL_TRUE
    );

    transfer_mat4("lightSpaceMatrix", currentLightSpaceMatrix);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(prog, "shadowMap"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapLeft);
    glUniform1i(glGetUniformLocation(prog, "headlightShadowMapLeft"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapRight);
    glUniform1i(glGetUniformLocation(prog, "headlightShadowMapRight"), 3);

    if (hasAlbedo)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.albedoTexture);
    }

    if (hasNormal)
    {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, material.normalTexture);
    }

    if (hasRoughness)
    {
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, material.roughnessTexture);
    }

    glBindVertexArray(obj.VAO);

    transfer_mat4("M", M);
    transfer_mat4("MVP", P * V * M);

    glDrawArrays(GL_TRIANGLES, 0, obj.Nv);

    glBindVertexArray(0);
}

void drawFloor(
    mat4 P,
    mat4 V,
    vec3 t,
    vec3 s
)
{
    dibujarObjetoConMaterial(
        suelo,
        P,
        V,
        t,
        s,
        vec3(1.0f),
        floorMaterial
    );
}

void drawWall(
    mat4 P,
    mat4 V,
    vec3 t,
    vec3 s
)
{
    dibujarObjetoConMaterial(
        pared,
        P,
        V,
        t,
        s,
        vec3(1.0f),
        wallMaterial
    );
}

static float dangerCeilingFade()
{
    const float fadeStart = 1.05f;
    const float fadeFull = 0.18f;

    float startY = worldHalfY - fadeStart;
    float fullY = worldHalfY - fadeFull;
    float t = glm::clamp(
        (playerY - startY) / (fullY - startY),
        0.0f,
        1.0f
    );

    return t * t * (3.0f - 2.0f * t);
}

static void renderDangerCeiling(
    mat4 P,
    mat4 V
)
{
    if (dangerCeilingProg == 0)
    {
        return;
    }

    float alpha = dangerCeilingFade();

    if (alpha <= 0.01f)
    {
        return;
    }

    vec3 ceilingPosition(
        0.0f,
        worldHalfY - 0.07f,
        playerZ + 4.8f
    );

    vec3 ceilingScale(
        worldHalfX * 2.08f,
        0.01f,
        24.5f
    );

    mat4 M =
        glm::translate(ceilingPosition) *
        glm::scale(ceilingScale);

    mat4 MVP = P * V * M;

    glUseProgram(dangerCeilingProg);

    glUniformMatrix4fv(
        glGetUniformLocation(dangerCeilingProg, "M"),
        1,
        GL_FALSE,
        glm::value_ptr(M)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(dangerCeilingProg, "MVP"),
        1,
        GL_FALSE,
        glm::value_ptr(MVP)
    );

    glUniform3f(
        glGetUniformLocation(dangerCeilingProg, "cameraPosition"),
        pos_obs.x,
        pos_obs.y,
        pos_obs.z
    );

    glUniform1f(
        glGetUniformLocation(dangerCeilingProg, "dangerAlpha"),
        alpha
    );

    glUniform1f(
        glGetUniformLocation(dangerCeilingProg, "currentTime"),
        currentTime
    );

    glUniform1f(
        glGetUniformLocation(dangerCeilingProg, "fogDensity"),
        fogDensity
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(suelo.VAO);

    glDrawArrays(
        GL_TRIANGLES,
        0,
        suelo.Nv
    );

    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUseProgram(prog);
}

void drawSunSphere(
    glm::mat4 P,
    glm::mat4 V,
    glm::mat4 M
)
{
    glUseProgram(sunProg);

    glm::mat4 MVP = P * V * M;

    glUniformMatrix4fv(
        glGetUniformLocation(sunProg, "MVP"),
        1,
        GL_FALSE,
        &MVP[0][0]
    );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sunTexture);

    glUniform1i(
        glGetUniformLocation(sunProg, "sunTexture"),
        1
    );

    glDisable(GL_CULL_FACE);

    glBindVertexArray(sun.VAO);

    glDrawArrays(
        GL_TRIANGLES,
        0,
        sun.Nv
    );

    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
    glUseProgram(prog);
}

void renderScene() {
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glViewport(0, 0, ANCHO, ALTO);

    beginFrame();

    updateGame();

    if (isGamePlaying())
    {
        updateSky();

        updateSun();
    }

    updateLighting();

    mat4 P = perspective(
        glm::radians(fov),
        aspect,
        nearPlane,
        farPlane
    );

    mat4 V = lookAt(
        pos_obs,
        target,
        up
    );

    updateCamera(P, V);

    // Primer pase: renderizamos profundidad desde la luz
    if (isGamePlaying())
    {
        renderShadowPass();
        renderHeadlightShadowPassLeft();
        renderHeadlightShadowPassRight();
    }

    //volver al framebuffer de escena
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glViewport(0, 0, ANCHO, ALTO);

    // Segundo pase: volvemos a limpiar la pantalla normal
    beginFrame();

    glUseProgram(prog);

    transfer_mat4("lightSpaceMatrix", currentLightSpaceMatrix);
    transfer_mat4("headlightMatrixLeft", headlightMatrixLeft);
    transfer_mat4("headlightMatrixRight", headlightMatrixRight);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapLeft);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D,headlightShadowMapRight);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, headlightCookieTexture);

    glUniform1i(glGetUniformLocation(prog, "shadowMap"), 1);
    glUniform1i(glGetUniformLocation(prog, "headlightShadowMapLeft"), 2);
    glUniform1i(glGetUniformLocation(prog, "headlightShadowMapRight"), 3);
    glUniform1i(glGetUniformLocation(prog, "useShadowMap"), GL_TRUE);

    if (isGamePlaying())
    {
        updateFog();
    }

    transferFogUniforms();

    renderFloor(P, V);

    transferFogUniforms();
    renderSideWalls(P, V);

    if (sunPosition.y > 0.0f) {
        renderSun(P, V);
        glUseProgram(prog);
    }

    transferFogUniforms();
    dibujarJugador(P, V);

    transferFogUniforms();
    renderObstacles(P, V);

    renderDangerCeiling(P, V);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, ANCHO, ALTO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(screenProg);
    glDisable(GL_DEPTH_TEST);

    // =========================================
    // Reconstrucción world-space para postprocesado volumétrico
    // =========================================

    glm::mat4 inverseProjection = glm::inverse(P);
    glm::mat4 inverseView = glm::inverse(V);

    glUniformMatrix4fv(
        glGetUniformLocation(screenProg, "inverseProjection"),
        1,
        GL_FALSE,
        glm::value_ptr(inverseProjection)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(
            screenProg,
            "inverseView"
        ),
        1,
        GL_FALSE,
        glm::value_ptr(inverseView)
    );

    glUniform1f(
        glGetUniformLocation(
            screenProg,
            "pauseGrayFactor"
        ),
        isGamePaused() ? 1.0f : 0.0f
    );

    glUniform3f(
        glGetUniformLocation(
            screenProg,
            "cameraPosition"
        ),
        pos_obs.x,
        pos_obs.y,
        pos_obs.z
    );

    glUniform1f(
        glGetUniformLocation(
            screenProg,
            "fogDensity"
        ),
        fogDensity
    );

    glUniform3f(
        glGetUniformLocation(
            screenProg,
            "spotlightLeftPos"
        ),
        spotlightLeftPos.x,
        spotlightLeftPos.y,
        spotlightLeftPos.z
    );

    glUniform3f(
        glGetUniformLocation(
            screenProg,
            "spotlightRightPos"
        ),
        spotlightRightPos.x,
        spotlightRightPos.y,
        spotlightRightPos.z
    );

    glUniform3f(
        glGetUniformLocation(
            screenProg,
            "spotlightDirection"
        ),
        spotlightDirection.x,
        spotlightDirection.y,
        spotlightDirection.z
    );

    glUniform3f(
        glGetUniformLocation(
            screenProg,
            "spotLightColor"
        ),
        spotLightColor.x,
        spotLightColor.y,
        spotLightColor.z
    );

    glUniform1f(
        glGetUniformLocation(
            screenProg,
            "spotlightCutoff"
        ),
        glm::cos(glm::radians(10.0f))
    );

    glUniform1f(
        glGetUniformLocation(
            screenProg,
            "spotlightOuterCutoff"
        ),
        glm::cos(glm::radians(18.0f))
    );

    glUniform1f(
        glGetUniformLocation(
            screenProg,
            "headlightsFactor"
        ),
        headlightsFactor
    );

    transferMat4ToProgram(
        screenProg,
        "headlightMatrixLeft",
        headlightMatrixLeft
    );

    glUniform1f(
        glGetUniformLocation(
           screenProg,
            "daylightFactor"
        ),
        daylightFactor
    );

    transferMat4ToProgram(
        screenProg,
        "headlightMatrixRight",
        headlightMatrixRight
    );

    // =========================================

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColorTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapLeft);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, headlightShadowMapRight);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, headlightCookieTexture);

    glUniform1i(glGetUniformLocation(screenProg, "screenTexture"), 0);
    glUniform1i(glGetUniformLocation(screenProg, "depthTexture"), 1);
    glUniform1i(glGetUniformLocation(screenProg, "headlightShadowMapLeft"), 2);
    glUniform1i(glGetUniformLocation(screenProg, "headlightShadowMapRight"), 3);
    glUniform1i(glGetUniformLocation(screenProg, "spotlightCookie"), 4);
    glBindVertexArray(screenQuadVAO);

    glDrawArrays(
        GL_TRIANGLES,
        0,
        6
    );

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void initViewport()
{
    int width, height;

    glfwGetFramebufferSize(
        window,
        &width,
        &height
    );

    ANCHO = width;
    ALTO = height;

    glViewport(
        0,
        0,
        width,
        height
    );
}

void initScreenQuad()
{
    float quadVertices[] =
    {
        // pos      // uv

        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);

    glBindVertexArray(screenQuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(quadVertices),
        quadVertices,
        GL_STATIC_DRAW
    );

    // position
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    // uv
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)(2 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void initMeshes()
{
    cubo  = crear_caja_texturizada();
    suelo = crear_suelo();
    pared = crear_pared();
    sun = cargar_modelo_gltf("../assets/models/sun/sun.glb", false);
    avion = cargar_modelo_gltf("../assets/models/plane/low_poly_plane.glb", true);
}

GLuint createShaderProgram(
    const char* vertexShader,
    const char* fragmentShader
)
{
    GLuint vertexID =
        compilar_shader(
            vertexShader,
            GL_VERTEX_SHADER
        );

    GLuint fragmentID =
        compilar_shader(
            fragmentShader,
            GL_FRAGMENT_SHADER
        );

    GLuint program =
        glCreateProgram();

    glAttachShader(program, vertexID);
    glAttachShader(program, fragmentID);

    glLinkProgram(program);

    check_errores_programa(program);

    glDetachShader(program, vertexID);
    glDeleteShader(vertexID);

    glDetachShader(program, fragmentID);
    glDeleteShader(fragmentID);

    return program;
}

void initShaders()
{
    prog =
        createShaderProgram(
            vertex_prog,
            fragment_prog
        );

    sunProg =
        createShaderProgram(
            sun_vertex_prog,
            sun_fragment_prog
        );

    shadowProg =
        createShaderProgram(
            depth_vertex_prog,
            depth_fragment_prog
        );

    screenProg =
        createShaderProgram(
            screen_vertex_prog,
            screen_fragment_prog
        );

    dangerCeilingProg =
        createShaderProgram(
            danger_ceiling_vertex_prog,
            danger_ceiling_fragment_prog
        );

}

void initMainShaderUniforms()
{
    glUseProgram(prog);

    glUniform1i(
        glGetUniformLocation(prog, "diffuseTexture"),
        0
    );

    glUniform1i(
        glGetUniformLocation(prog, "shadowMap"),
        1
    );

    glUniform1i(
        glGetUniformLocation(prog, "headlightShadowMapLeft"),
        2
    );

    glUniform1i(
        glGetUniformLocation(prog, "headlightShadowMapRight"),
        3
    );

    glUniform1i(
        glGetUniformLocation(prog, "spotlightCookie"),
        4
    );

    glUniform1i(
        glGetUniformLocation(prog, "normalTexture"),
        5
    );

    glUniform1i(
        glGetUniformLocation(prog, "roughnessTexture"),
        6
    );

    glUniform1i(
        glGetUniformLocation(prog, "useTexture"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useVertexColor"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useBumpMap"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useNormalMap"),
        GL_FALSE
    );

    glUniform1i(
        glGetUniformLocation(prog, "useRoughnessMap"),
        GL_FALSE
    );

    glUniform1f(
        glGetUniformLocation(prog, "normalMapStrength"),
        1.0f
    );

    glUniform1f(
        glGetUniformLocation(prog, "roughnessFactor"),
        0.55f
    );

    glUniform1f(
        glGetUniformLocation(prog, "bumpStrength"),
        0.0f
    );

    glUniform1i(
        glGetUniformLocation(prog, "useShadowMap"),
        GL_TRUE
    );
}

void initSceneSystems()
{
    initShadowMap();
    initHeadlightShadowMaps();
    initSceneFramebuffer();
    initScreenQuad();
    initFog();
}

void initTextures()
{
    floorTexture = cargar_textura("../assets/textures/floor_diffuse.jpg", GL_TEXTURE0);
    wallTexture = cargar_textura("../assets/textures/wall_diffuse.jpg", GL_TEXTURE0);
    sunTexture = cargar_textura("../assets/textures/sun_diffuse.png", GL_TEXTURE0);
    planeTexture = cargar_textura("../assets/textures/plane_diffuse.png", GL_TEXTURE0);
    headlightCookieTexture = cargar_textura("../assets/textures/headlight_cookie.jpg", GL_TEXTURE4);

    floorMaterial.albedoTexture =
        cargarTexturaSegura(
            "../assets/materials/floor/albedo.jpg",
            GL_TEXTURE0,
            "floor albedo"
        );

    floorMaterial.normalTexture =
        cargarTexturaSegura(
            "../assets/materials/floor/normal.png",
            GL_TEXTURE5,
            "floor normal"
        );

    floorMaterial.roughnessTexture =
        cargarTexturaSegura(
            "../assets/materials/floor/roughness.png",
            GL_TEXTURE6,
            "floor roughness"
        );

    floorMaterial.normalStrength = 1.15f;
    floorMaterial.roughnessFactor = 0.80f;

    if (floorMaterial.albedoTexture == 0)
    {
        floorMaterial.albedoTexture = floorTexture;
    }

    wallMaterial.albedoTexture =
        cargarTexturaSegura(
            "../assets/materials/wall/albedo.jpg",
            GL_TEXTURE0,
            "wall albedo"
        );

    wallMaterial.normalTexture =
        cargarTexturaSegura(
            "../assets/materials/wall/normal.png",
            GL_TEXTURE5,
            "wall normal"
        );

    wallMaterial.roughnessTexture =
        cargarTexturaSegura(
            "../assets/materials/wall/roughness.png",
            GL_TEXTURE6,
            "wall roughness"
        );

    wallMaterial.normalStrength = 1.30f;
    wallMaterial.roughnessFactor = 0.88f;

    if (wallMaterial.albedoTexture == 0)
    {
        wallMaterial.albedoTexture = wallTexture;
    }
}

void initOpenGLState()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(
        0.0f,
        0.0f,
        0.0f,
        1.0f
    );
}

void initScene()
{
    initViewport();

    initMeshes();

    initShaders();

    initMainShaderUniforms();

    initSceneSystems();

    initTextures();

    initOpenGLState();
}
