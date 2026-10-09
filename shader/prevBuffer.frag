#version 430 core

uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D DisplayOutput;

layout (location = 0) out vec4 prevFirstMoment;
layout (location = 1) out vec3 prevSecondMoment;
layout (location = 2) out vec4 gDepth;
layout (location = 3) out float gNormal;

in vec2 TexCoords;

void main() {
    
}