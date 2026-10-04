#version 430 core

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;

uniform mat4 view;
uniform mat4 prevView;

uniform mat4 projection;
uniform mat4 prevProjection;

uniform mat3 normalMatrix;

void main () {
    vec4 ClipPos = projection * view * model * vec4(aPos, 1.0f);
    
    gl_Position = ClipPos;
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalMatrix * aNormal;
    TexCoords = aTexCoords;
}