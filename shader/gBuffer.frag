#version 430 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

layout (location = 0) out vec4 gBuffers;

uniform int albedoTextureIdx;
uniform int roughnessTextureIdx;

uniform vec3 Albedo;
uniform float Roughness;

uniform bool isDielectric;
uniform float IOR;

uniform sampler2DArray MeshTextures;

#define gPositionLayer 0
#define gNormalLayer 1
#define gAlbedoLayer 2
#define gRoughnessLayer 3
#define gIsDielectricLayer 4
#define gIORLayer 5

uniform int u_LayerIndex;

void main() {
    if (u_LayerIndex == gPositionLayer) {
        gBuffers = vec4(FragPos, 1.0);
    }
    
    if (u_LayerIndex == gNormalLayer) {
        gBuffers = vec4(normalize(Normal), 1.0);
    }

    if (u_LayerIndex == gAlbedoLayer) {
        if (albedoTextureIdx >= 0)
            gBuffers = vec4(textureLod(MeshTextures, vec3(TexCoords, float(albedoTextureIdx)), 0.0).rgb, 1.0);
        else
            gBuffers = vec4(Albedo, 1.0);
    }

    if (u_LayerIndex == gRoughnessLayer) {
        if (roughnessTextureIdx >= 0)
            gBuffers = vec4(vec3(textureLod(MeshTextures, vec3(TexCoords, float(roughnessTextureIdx)), 0.0).r), 1.0);
        else
            gBuffers = vec4(vec3(Roughness), 1.0);
    }

    if (u_LayerIndex == gIsDielectricLayer) {
        gBuffers = isDielectric ? vec4(vec3(1.0),1.0) : vec4(vec3(0.0), 1.0);
    }

    if (u_LayerIndex == gIORLayer) {
        gBuffers = vec4((vec3(IOR)), 1.0);
    }
}