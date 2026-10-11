#version 430 core

uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D DisplayOutput;

layout (location = 0) out float prevFirstMoment;
layout (location = 1) out float prevSecondMoment;
layout (location = 2) out float prevDepth;
layout (location = 3) out vec4 prevNormal;

in vec2 TexCoords;

void main() {
    float Luminance = dot(vec3(0.2126, 0.7152, 0.0722), texture(DisplayOutput, TexCoords).rgb); //weights are calculated by the sensitivity of each colour channel to a human's eyes
    prevFirstMoment = Luminance;
    prevSecondMoment = pow(Luminance, 2.0);
    prevDepth = texture(gDepth, TexCoords).r;
    prevNormal = texture(gNormal, TexCoords);
}