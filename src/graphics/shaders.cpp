#include "graphics/shaders.h"

#define GLSL(src) "#version 330 core\n" #src

const char* vertex_prog = GLSL(
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 vertexColor;

uniform mat4 MVP = mat4(1.0f);
uniform mat4 M = mat4(1.0f);
uniform mat4 lightSpaceMatrix = mat4(1.0f);
uniform mat4 headlightMatrixLeft;
uniform mat4 headlightMatrixRight;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 VertexColor;
out vec4 FragPosLightSpace;
out vec4 FragPosHeadlightLeft;
out vec4 FragPosHeadlightRight;

void main()
{
    FragPos = vec3(M * vec4(pos, 1.0f));
    Normal = mat3(transpose(inverse(M))) * normal;
    TexCoord = texCoord;
    VertexColor = vertexColor;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0f);
    FragPosHeadlightLeft = headlightMatrixLeft * vec4(FragPos, 1.0f);
    FragPosHeadlightRight = headlightMatrixRight * vec4(FragPos, 1.0f);
    gl_Position = MVP * vec4(pos, 1.0f);
}
);

const char* fragment_prog = GLSL(
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 VertexColor;
in vec4 FragPosLightSpace;
in vec4 FragPosHeadlightLeft;
in vec4 FragPosHeadlightRight;

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform vec3 fillLightPos;
uniform vec3 fillLightColor;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D roughnessTexture;
uniform bool useTexture;
uniform bool useVertexColor;

uniform bool useBumpMap;
uniform bool useNormalMap;
uniform bool useRoughnessMap;
uniform float bumpStrength;
uniform float normalMapStrength;
uniform float roughnessFactor;
uniform sampler2D shadowMap;
uniform sampler2D headlightShadowMapLeft;
uniform sampler2D headlightShadowMapRight;
uniform bool useShadowMap;

uniform sampler2D spotlightCookie;
uniform vec3 spotLightColor;
uniform float spotlightCutoff;
uniform float spotlightOuterCutoff;
uniform float daylightFactor;
uniform float headlightsFactor;

uniform vec3 spotlightLeftPos;
uniform vec3 spotlightRightPos;
uniform vec3 spotlightDirection;

uniform bool useFog = false;
uniform vec3 fogColor = vec3(0.70f, 0.74f, 0.76f);
uniform float fogDensity = 0.0f;
uniform float fogStart = 0.0f;
uniform float fogEnd = 4.5f;
out vec4 outputColor;

float luminance(vec3 c)
{
    return dot(c, vec3(0.299f, 0.587f, 0.114f));
}

vec3 calcularNormalConBump(vec3 normalBase, vec3 fragPos, vec2 uv)
{
    vec3 N = normalize(normalBase);

    vec3 dp1 = dFdx(fragPos);
    vec3 dp2 = dFdy(fragPos);

    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 T = dp1 * duv2.y - dp2 * duv1.y;
    vec3 B = -dp1 * duv2.x + dp2 * duv1.x;

    if (length(T) < 0.0001f || length(B) < 0.0001f)
    {
        return N;
    }

    T = normalize(T);
    B = normalize(B);

    vec2 texSize = vec2(textureSize(diffuseTexture, 0));

    if (texSize.x <= 0.0f || texSize.y <= 0.0f)
    {
        return N;
    }

    // Usamos un radio algo mayor que 1 texel para que el relieve sea visible.
    vec2 texel = 3.0f / texSize;

    float hC = luminance(texture(diffuseTexture, uv).rgb);

    float hL = luminance(texture(diffuseTexture, uv - vec2(texel.x, 0.0f)).rgb);
    float hR = luminance(texture(diffuseTexture, uv + vec2(texel.x, 0.0f)).rgb);
    float hD = luminance(texture(diffuseTexture, uv - vec2(0.0f, texel.y)).rgb);
    float hU = luminance(texture(diffuseTexture, uv + vec2(0.0f, texel.y)).rgb);

    // Centramos la altura para exagerar piedras/grietas.
    float dHdU = (hR - hL) * bumpStrength;
    float dHdV = (hU - hD) * bumpStrength;

    vec3 tangentNormal = normalize(vec3(-dHdU, -dHdV, 1.0f));

    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

mat3 calcularTBN(vec3 normalBase, vec3 fragPos, vec2 uv)
{
    vec3 N = normalize(normalBase);

    vec3 dp1 = dFdx(fragPos);
    vec3 dp2 = dFdy(fragPos);

    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 T = dp1 * duv2.y - dp2 * duv1.y;
    vec3 B = -dp1 * duv2.x + dp2 * duv1.x;

    if (length(T) < 0.0001f || length(B) < 0.0001f)
    {
        vec3 up = abs(N.y) < 0.999f ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);
        T = normalize(cross(up, N));
        B = normalize(cross(N, T));
    }
    else
    {
        T = normalize(T);
        B = normalize(B);
    }

    return mat3(T, B, N);
}

vec3 calcularNormalConNormalMap(vec3 normalBase, vec3 fragPos, vec2 uv)
{
    vec3 N = normalize(normalBase);
    vec3 tangentNormal = texture(normalTexture, uv).xyz * 2.0f - 1.0f;

    // Permite suavizar normal maps demasiado agresivos o conversiones dudosas.
    tangentNormal.xy *= normalMapStrength;
    tangentNormal = normalize(tangentNormal);

    return normalize(calcularTBN(N, fragPos, uv) * tangentNormal);
}

float calcularFactorNiebla(vec3 fragPos)
{
    float distanceToCamera = length(viewPos - fragPos);
    float distanceFactor = smoothstep(fogStart, fogEnd, distanceToCamera);
    float densityFactor = clamp(fogDensity / 0.60f, 0.0f, 1.0f);
    densityFactor = densityFactor * densityFactor * (3.0f - 2.0f * densityFactor);
    float exponentialFog = 1.0f - exp(-fogDensity * distanceToCamera * 0.55f);
    float fogFactor = distanceFactor * densityFactor * exponentialFog * 1.22f;

    return clamp(fogFactor, 0.0f, 0.78f);
}

float calcularIntensidadFaro(
    vec3 lightPos,
    vec3 lightDir,
    vec3 fragPos,
    float horizontalOffset
)
{
    vec3 L = normalize(lightPos - fragPos);

    vec3 ellipticalL =
        normalize(
            vec3(
                L.x + horizontalOffset,
                L.y * 1.7,
                L.z
            )
        );

    float theta = dot(ellipticalL, normalize(-lightDir));
    float verticalFade = clamp(1.0 - abs(L.y) * 1.4, 0.0, 1.0);
    float intensity = smoothstep(spotlightOuterCutoff, spotlightCutoff, theta) * verticalFade;

    return intensity;
}

float calcularSombraBase(
    vec4 fragPosLightSpace,
    vec3 normal,
    vec3 lightDir,
    sampler2D depthTexture,
    float outsideShadow
)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Pasar de coordenadas [-1, 1] a [0, 1]
    projCoords = projCoords * 0.5f + 0.5f;

    // Si está fuera del mapa de sombras, no aplicamos sombra
    if (projCoords.z > 1.0f)
        {
        return outsideShadow;
    }

    if (projCoords.x < 0.0f || projCoords.x > 1.0f ||
        projCoords.y < 0.0f || projCoords.y > 1.0f)
    {
        return outsideShadow;
    }

    float currentDepth = projCoords.z;

    // Bias para reducir shadow acne
    float bias = max(0.004f * (1.0f - dot(normal, lightDir)), 0.0015f);

    float shadow = 0.0f;

    vec2 texelSize = 1.0f / vec2(textureSize(depthTexture, 0));

    // PCF 3x3 para suavizar la sombra
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closestDepth =
                texture(
                    depthTexture,
                    projCoords.xy + vec2(x, y) * texelSize
                ).r;

            shadow += currentDepth - bias > closestDepth ? 1.0f : 0.0f;
        }
    }

    shadow /= 9.0f;

    return shadow;
}

float calcularSombra(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, sampler2D depthTexture)
{
    return calcularSombraBase(
        fragPosLightSpace,
        normal,
        lightDir,
        depthTexture,
        0.0f
    );
}

float calcularSombraFaro(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, sampler2D depthTexture)
{
    return calcularSombraBase(
        fragPosLightSpace,
        normal,
        lightDir,
        depthTexture,
        1.0f
    );
}

float sampleSpotlightCookie(
    vec4 fragPosLightSpace
)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 ||
        projCoords.x > 1.0 ||
        projCoords.y < 0.0 ||
        projCoords.y > 1.0)
    {
        return 0.0;
    }

    float cookie = 0.0;
    vec2 texel = 1.0 / vec2(textureSize(spotlightCookie, 0));

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            cookie += texture(spotlightCookie, projCoords.xy + vec2(x, y) * texel).r;
        }
    }

    cookie /= 9.0;
    return cookie;
}

vec3 calcularSpotlight(
    vec3 lightPos,
    vec3 lightDir,
    vec3 fragPos,
    vec3 normal,
    vec3 viewDir,
    float horizontalOffset,
    vec4 fragPosLightSpace
)
{
    vec3 L = normalize(lightPos - fragPos);

    vec3 ellipticalL =
    normalize(
        vec3(
            L.x + horizontalOffset,
            L.y * 1.7,
            L.z
        )
    );
    float theta = dot(ellipticalL, normalize(-lightDir));
    float intensity = calcularIntensidadFaro(lightPos, lightDir, fragPos, horizontalOffset);
    float lateralFalloff = pow(intensity, 3.4);
    float hotspot = pow(max(theta, 0.0), 28.0);
    float angularDiffuse = pow(max(dot(normal, L), 0.0), 1.35);
    vec3 diffuse = angularDiffuse * spotLightColor;
    vec3 reflectDir = reflect(-L, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = spec * spotLightColor * 0.012;
    float distance = length(lightPos - fragPos);
    float attenuation =
        1.0 /
        (
            1.0 +
            0.22 * distance +
            0.16 * distance * distance
        );
    float cookie = sampleSpotlightCookie(fragPosLightSpace);
    float cookieInfluence = mix(0.82, 1.0, cookie);

    return
    (diffuse + specular) *
    lateralFalloff *
    attenuation *
    cookieInfluence *
    (0.22 + hotspot * 0.55);
}

void main()
{
    // =========================================
    // Color base
    // =========================================

    vec3 baseColor = objectColor;

    if (useTexture)
    {
        baseColor *= texture(diffuseTexture, TexCoord).rgb;
    }

    if (useVertexColor)
    {
        baseColor *= VertexColor;
    }

    // =========================================
    // Normal
    // =========================================

    vec3 norm = normalize(Normal);

    // =========================================
    // Bump mapping
    // =========================================

    if (useBumpMap && useTexture)
    {
        norm = calcularNormalConBump(Normal, FragPos, TexCoord);
    }

    if (useNormalMap)
    {
        norm = calcularNormalConNormalMap(Normal, FragPos, TexCoord);
    }

    // =========================================
    // Luz solar direccional
    // =========================================

    vec3 L = normalize(-lightDir);

    // =========================================
    // Ambiente
    // =========================================

    float ambientStrength =
        mix(
            0.003f,
            0.10f,
            daylightFactor
        );
    vec3 ambient = ambientStrength * lightColor;

    // =========================================
    // Difusa
    // =========================================

    float diff = max(dot(norm, L), 0.0f);
    vec3 diffuse = diff * 1.35f * lightColor * daylightFactor;

    // =========================================
    // Luz secundaria
    // =========================================

    vec3 fillDir = normalize(fillLightPos - FragPos);
    float fillDiff = max(dot(norm, fillDir), 0.0f);
    vec3 fillDiffuse = fillDiff * fillLightColor * 0.35f;

    // =========================================
    // Especular
    // =========================================

    float materialRoughness = clamp(roughnessFactor, 0.04f, 1.0f);

    if (useRoughnessMap)
    {
        materialRoughness =
            clamp(
                texture(roughnessTexture, TexCoord).g * roughnessFactor,
                0.04f,
                1.0f
            );
    }

    float specularStrength = mix(0.75f, 0.10f, materialRoughness);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-L, norm);

    float spec =
        pow(
            max(dot(viewDir, reflectDir), 0.0f),
            mix(96.0f, 8.0f, materialRoughness)
    );
    vec3 specular = specularStrength * spec * lightColor * daylightFactor;

    // =========================================
    // Faros nocturnos
    // =========================================

    vec3 spotlightLeft =
        calcularSpotlight(
            spotlightLeftPos,
            spotlightDirection,
            FragPos,
            norm,
            viewDir,
            0.08,
            FragPosHeadlightLeft
        );

    float spotlightShadowLeft =
        calcularSombraFaro(
            FragPosHeadlightLeft,
            norm,
            normalize(spotlightLeftPos - FragPos),
            headlightShadowMapLeft
        );

    vec3 spotlightRight =
        calcularSpotlight(
            spotlightRightPos,
            spotlightDirection,
            FragPos,
            norm,
            viewDir,
            -0.08,
            FragPosHeadlightRight
        );

    float spotlightShadowRight =
        calcularSombraFaro(
            FragPosHeadlightRight,
            norm,
            normalize(spotlightRightPos - FragPos),
            headlightShadowMapRight
        );

    // Intensidad nocturna
    float nightFactor =
        clamp(
            1.0f - daylightFactor,
            0.0f,
            1.0f
        );

    float directNightWeight =
        mix(
            0.08f,
            0.68f,
            pow(nightFactor, 1.2f)
        );

    float fogNorm =
        clamp(
            (fogDensity - 0.30f) / 0.20f,
            0.0f,
            1.0f
        );

    float fogSurfaceAtten =
        mix(
            1.0f,
            0.55f,
            clamp(fogDensity / 0.60f, 0.0f, 1.0f)
        );

    float directHeadlightWeight =
        headlightsFactor *
        directNightWeight *
        fogSurfaceAtten;

    float leftDistanceToLight = length(spotlightLeftPos - FragPos);
    float rightDistanceToLight = length(spotlightRightPos - FragPos);
    float fogExtinction = mix(0.0f, 0.12f, fogNorm);

    float leftFogDistanceAtten =
        exp(-fogExtinction * leftDistanceToLight);

    float rightFogDistanceAtten =
        exp(-fogExtinction * rightDistanceToLight);

    float leftNearBoost =
        1.0f +
        fogNorm *
        0.35f *
        (1.0f - smoothstep(2.5f, 7.0f, leftDistanceToLight));

    float rightNearBoost =
        1.0f +
        fogNorm *
        0.35f *
        (1.0f - smoothstep(2.5f, 7.0f, rightDistanceToLight));

    float leftFogDistanceWeight =
        leftFogDistanceAtten *
        leftNearBoost;

    float rightFogDistanceWeight =
        rightFogDistanceAtten *
        rightNearBoost;

    spotlightLeft *=
        (1.0f - spotlightShadowLeft) *
        directHeadlightWeight *
        leftFogDistanceWeight;

    spotlightRight *=
        (1.0f - spotlightShadowRight) *
        directHeadlightWeight *
        rightFogDistanceWeight;

    // =========================================
    // Sombras
    // =========================================

    float shadow = 0.0f;

    if (useShadowMap && daylightFactor > 0.01f)
    {
        shadow =
            calcularSombra(
                FragPosLightSpace,
                norm,
                L,
                shadowMap
            );
    }

    // =========================================
    // Iluminación final
    // =========================================

    vec3 lighting =
        ambient +

        // La sombra SOLO afecta a luz solar
        (1.0f - shadow * 0.65f) *
        (
            diffuse +
            specular
        )

        +

        // Fill light SIN sombras
        fillDiffuse

        +

        // Faros con shadow maps
        spotlightLeft +
        spotlightRight;

    // =========================================
    // Resultado final
    // =========================================

    vec3 result = lighting * baseColor;

    if (useFog)
    {
        float fogFactor = calcularFactorNiebla(FragPos);

        result =
            mix(
                result,
                fogColor,
                fogFactor
            );
    }

    outputColor =
        vec4(result, 1.0f);
}
);
