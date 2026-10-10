#version 430 core

uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D DisplayOutput;

layout (location = 0) out vec4 prevFirstMoment;
layout (location = 1) out vec4 prevSecondMoment;
layout (location = 2) out float prevDepth;
layout (location = 3) out vec4 prevNormal;

in vec2 TexCoords;

void main() {
    prevFirstMoment = texture(DisplayOutput, TexCoords);
    prevSecondMoment = pow(texture(DisplayOutput, TexCoords), 2);
    prevDepth = texture(gDepth, TexCoords);
    prevNormal = texture(gNormal, TexCoords);
}