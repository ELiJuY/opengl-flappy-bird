#include "graphics/sunShaders.h"

#define GLSL(src) "#version 330 core\n" #src

const char* sun_vertex_prog = GLSL(
layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

uniform mat4 MVP;

out vec2 TexCoord;

void main()
{
    TexCoord = uv;

    gl_Position =
        MVP * vec4(pos, 1.0);
}
);

const char* sun_fragment_prog = GLSL(
in vec2 TexCoord;

uniform sampler2D sunTexture;

out vec4 outputColor;

void main()
{
    vec3 tex = texture(sunTexture, TexCoord).rgb;

    // Intensidad emissive
    tex *= 1.8;

    outputColor = vec4(tex, 1.0);
}
);