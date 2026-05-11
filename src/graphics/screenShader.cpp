#include "graphics/screenShader.h"

#define GLSL(src) "#version 330 core\n" #src

const char* screen_vertex_prog = GLSL(
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

out vec2 TexCoord;

void main()
{
    TexCoord = uv;

    gl_Position =
        vec4(pos, 0.0f, 1.0f);
}
);

const char* screen_fragment_prog = GLSL(
in vec2 TexCoord;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform sampler2D spotlightCookie;

uniform mat4 inverseProjection;
uniform mat4 inverseView;

uniform vec3 cameraPosition;

uniform float headlightsFactor;
uniform float daylightFactor;
uniform float fogDensity;
uniform float pauseGrayFactor;

uniform vec3 spotlightLeftPos;
uniform vec3 spotlightRightPos;
uniform vec3 spotlightDirection;
uniform vec3 spotLightColor;
uniform float spotlightCutoff;
uniform float spotlightOuterCutoff;

uniform mat4 headlightMatrixLeft;
uniform mat4 headlightMatrixRight;
uniform sampler2D headlightShadowMapLeft;
uniform sampler2D headlightShadowMapRight;

out vec4 outputColor;

vec3 reconstructWorldPosition(vec2 uv,float depth)
{
    float z = depth * 2.0 - 1.0;
    vec4 clipSpace =
        vec4(
            uv * 2.0 - 1.0,
            z,
            1.0
        );

    vec4 viewSpace = inverseProjection * clipSpace;
    viewSpace /= viewSpace.w;
    vec4 worldSpace = inverseView * viewSpace;

    return worldSpace.xyz;
}

float evaluateHeadlightCone(
    vec3 lightPos,
    vec3 lightDir,
    vec3 samplePos,
    float horizontalOffset
)
{
    vec3 toLight = normalize(lightPos - samplePos);
    vec3 ellipticalL =
        normalize(
            vec3(
                toLight.x + horizontalOffset,
                toLight.y * 1.7,
                toLight.z
            )
        );

    float theta = dot(ellipticalL, normalize(-lightDir));
    float cone =
        smoothstep(
            spotlightOuterCutoff,
            spotlightCutoff,
            theta
        );

    // Endurece el nucleo para evitar cono "algodon".
    cone = pow(cone, 2.4);

    float verticalFade =
        clamp(
            1.0 - abs(toLight.y) * 1.4,
            0.0,
            1.0
        );
    float distance = length(lightPos - samplePos);

    float attenuation = 1.0 /
        (1.0 + 0.055 * distance + 0.012 * distance * distance);

    return cone * verticalFade * attenuation;
}

float sampleSpotlightCookie(
    vec3 worldPos,
    mat4 lightMatrix
)
{
    vec4 lightSpace = lightMatrix * vec4(worldPos, 1.0);

    if (lightSpace.w <= 0.0)
    {
        return 0.0;
    }

    vec3 projCoords = lightSpace.xyz / lightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 ||
        projCoords.x > 1.0 ||
        projCoords.y < 0.0 ||
        projCoords.y > 1.0 ||
        projCoords.z < 0.0 ||
        projCoords.z > 1.0)
    {
        return 0.0;
    }

    return texture(spotlightCookie, projCoords.xy).r;
}

float sampleVolumetricShadow(
    sampler2D shadowMap,
    mat4 lightMatrix,
    vec3 worldPos
)
{
    vec4 lightSpace = lightMatrix * vec4(worldPos, 1.0);

    if (lightSpace.w <= 0.0)
    {
        return 0.0;
    }

    vec3 projCoords = lightSpace.xyz / lightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 ||
        projCoords.x > 1.0 ||
        projCoords.y < 0.0 ||
        projCoords.y > 1.0 ||
        projCoords.z < 0.0 ||
        projCoords.z > 1.0)
    {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float bias = 0.0025;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float visibility = 0.0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closestDepth =
                texture(
                    shadowMap,
                    projCoords.xy + vec2(x, y) * texelSize
                ).r;

            visibility +=
                (currentDepth - bias <= closestDepth) ? 1.0 : 0.0;
        }
    }

    return visibility / 9.0;
}

float raymarchFog(
    vec3 rayOrigin,
    vec3 rayDir,
    float maxDistance
)
{
    if (maxDistance <= 0.0 || headlightsFactor < 0.001)
    {
        return 0.0;
    }

    float fogNorm = clamp((fogDensity - 0.18) / 0.42, 0.0, 1.0);
    float marchDistance = min(maxDistance, mix(10.0, 14.0, fogNorm));

    const int STEPS = 48;
    float stepSize = marchDistance / float(STEPS);
    float scattering = 0.0;
    float transmittance = 1.0;
    float nightFactor = clamp(1.0 - daylightFactor, 0.0, 1.0);
    float nightFogFactor = nightFactor * fogNorm;
    float nightVisibility = mix(0.42, 1.05, pow(nightFactor, 1.05));
    float fogBeamBoost =
        mix(1.05, 1.85, fogNorm) *
        mix(1.0, 0.62, nightFogFactor);
    float dryNightBoost =
        mix(1.0, 2.8, pow(nightFactor, 1.25)) *
        mix(1.0, 0.68, fogNorm);
    float forwardViewBoost =
        mix(
            0.82,
            1.24,
            pow(
                max(dot(normalize(spotlightDirection), rayDir), 0.0),
                1.35
            )
        );

    for (int i = 0; i < STEPS; ++i)
    {
        float dist = (float(i) + 0.5) * stepSize;
        vec3 samplePos = rayOrigin + rayDir * dist;
        float cameraFade = smoothstep(0.5, 2.2, dist);
        float farFade = 1.0 - smoothstep(marchDistance * 0.55, marchDistance, dist);
        float lightDistance =
            min(
                length(samplePos - spotlightLeftPos),
                length(samplePos - spotlightRightPos)
            );
        float localBeamWeight =
            mix(
                0.38,
                2.15,
                1.0 - smoothstep(1.2, mix(8.5, 11.5, fogNorm), lightDistance)
            );
        float atmosphericDensity = 0.0026;
        float fogContribution =
            clamp(fogDensity, 0.0, 1.0) *
            0.032 *
            mix(1.0, 0.18, nightFactor);
        float density = atmosphericDensity + fogContribution;

        float leftLight =
            evaluateHeadlightCone(
                spotlightLeftPos,
                spotlightDirection,
                samplePos,
                0.08
            );

        float rightLight =
            evaluateHeadlightCone(
                spotlightRightPos,
                spotlightDirection,
                samplePos,
                -0.08
            );

        float leftShadow =
            sampleVolumetricShadow(
                headlightShadowMapLeft,
                headlightMatrixLeft,
                samplePos
            );

        float rightShadow =
            sampleVolumetricShadow(
                headlightShadowMapRight,
                headlightMatrixRight,
                samplePos
            );

        float leftCookie =
            mix(
                0.82,
                1.0,
                sampleSpotlightCookie(samplePos, headlightMatrixLeft)
            );

        float rightCookie =
            mix(
                0.82,
                1.0,
                sampleSpotlightCookie(samplePos, headlightMatrixRight)
            );

        float lightContribution =
            leftLight * leftShadow * leftCookie +
            rightLight * rightShadow * rightCookie;

        float inScattering =
            density *
            lightContribution *
            stepSize *
            headlightsFactor *
            nightVisibility *
            fogBeamBoost *
            dryNightBoost *
            forwardViewBoost *
            cameraFade *
            farFade *
            localBeamWeight *
            1.45;

        scattering += inScattering * transmittance;
        transmittance *= exp(-density * stepSize * 0.6);
    }

    float nightFogLimit =
        mix(
            32.0,
            0.42,
            smoothstep(0.12, 0.85, nightFogFactor)
        );

    return min(scattering, nightFogLimit);
}


void main()
{
    vec3 sceneColor = texture(screenTexture, TexCoord).rgb;
    float depth = texture(depthTexture, TexCoord).r;
    bool hasSceneGeometry = depth < 0.9995;
    vec3 worldPos = reconstructWorldPosition(TexCoord, depth);
    vec3 rayVec = worldPos - cameraPosition;
    float distanceToPixel = length(rayVec);

    vec3 rayDir =
        distanceToPixel > 0.0001
        ? rayVec / distanceToPixel
        : vec3(0.0, 0.0, 1.0);

    float volumetric =
        hasSceneGeometry
        ? raymarchFog(cameraPosition, rayDir, distanceToPixel)
        : 0.0;
    vec3 volumetricColor = spotLightColor * volumetric;
    volumetricColor = volumetricColor / (vec3(1.0) + volumetricColor);

    float sceneLuma = dot(sceneColor, vec3(0.299, 0.587, 0.114));
    float surfaceProtection =
        mix(
            1.0,
            0.38,
            smoothstep(0.25, 1.0, sceneLuma)
        );

    vec3 finalColor = sceneColor + volumetricColor * surfaceProtection;
    float gray = dot(finalColor, vec3(0.299, 0.587, 0.114));
    finalColor = mix(finalColor, vec3(gray), pauseGrayFactor);

    outputColor = vec4(finalColor, 1.0);
}
);
