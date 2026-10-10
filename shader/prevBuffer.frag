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
    vec4 Output = texture(DisplayOutput, TexCoords);
    prevFirstMoment = Output;
    prevSecondMoment = pow(Output, vec4(2.0));
    prevDepth = texture(gDepth, TexCoords).r;
    prevNormal = texture(gNormal, TexCoords);
}