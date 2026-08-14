#version 430 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out float gRoughness;

uniform int albedoTextureIdx;
uniform int roughnessTextureIdx;

uniform vec3 Albedo;
uniform float Roughness;

uniform sampler2DArray MeshTextures;

void main() {
    gPosition = FragPos;
    
    gNormal = normalize(Normal);

    if (albedoTextureIdx >= 0)
        gAlbedo = textureLod(MeshTextures, vec3(TexCoords, float(albedoTextureIdx)), 0.0).rgb;
    else
        gAlbedo = Albedo;

    gAlbedo = pow(gAlbedo, vec3(2.2));

    if (roughnessTextureIdx >= 0)
        gRoughness = textureLod(MeshTextures, vec3(TexCoords, float(roughnessTextureIdx)), 0.0).r;
    else
        gRoughness = Roughness;
}