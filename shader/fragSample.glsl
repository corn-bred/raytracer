#version 420 core

out vec4 FragColor;

uniform sampler2D TextureAccumulation;
in vec2 TexCoords;

void main () {
    FragColor = texture(TextureAccumulation,TexCoords);
}