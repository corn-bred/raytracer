#version 420 core

in vec2 TexCoords;

out vec4 FragColor;

uniform vec2 Dimensions;
uniform vec3 CamUp;
uniform vec3 CamFront;
uniform vec3 CamRight;
uniform float FOV;

void main () {
    vec2 Coords = (TexCoords + 1) / 2;
    vec2 PointLocal = Dimensions * TexCoords;
    vec3 Point = vec3(CamRight * TexCoords.x, CamUp * TexCoords.y, CamFront * );
    FragColor = vec4(TexCoords, 0.0, 1.0);
}