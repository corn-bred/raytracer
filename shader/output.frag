#version 430 core

out vec4 FragColor;

uniform sampler2D DisplayOutput;
in vec2 TexCoords;

void main () {
    FragColor = texture(DisplayOutput, TexCoords);
}