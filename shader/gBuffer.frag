#version 430 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out float gRoughness;

uniform int albedoTextureIdx;
uniform int roughnessTextureIdx;

uniform vec3 Albedo;
uniform float Roughness;

uniform bool isDielectric;
uniform float IOR;

uniform sampler2DArray MeshTextures;

void main() {
    gPosition = vec4(FragPos, isDielectric ? 1.0 : 0.0);
    
    gNormal = normalize(Normal);

    if (albedoTextureIdx >= 0)
        gAlbedo = vec4(textureLod(MeshTextures, vec3(TexCoords, float(albedoTextureIdx)), 0.0).rgb, IOR);
    else
        gAlbedo = vec4(Albedo, IOR);

    if (roughnessTextureIdx >= 0)
        gRoughness = textureLod(MeshTextures, vec3(TexCoords, float(roughnessTextureIdx)), 0.0).r;
    else
        gRoughness = Roughness;
}