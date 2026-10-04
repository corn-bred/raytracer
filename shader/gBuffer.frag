#version 430 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec2 MotionVectorUV;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out float gRoughness;
layout (location = 4) out float gIsDielectric;
layout (location = 5) out float gIOR;
layout (location = 6) out float gIsEmissor;
layout (location = 7) out vec2 gMotion;

uniform int albedoTextureIdx;
uniform int roughnessTextureIdx;

uniform vec3 Albedo;
uniform float Roughness;

uniform bool isDielectric;
uniform float IOR;

uniform bool isEmissor;

uniform sampler2DArray MeshTextures;

uniform mat4 prevView;
uniform mat4 prevProjection;
uniform vec2 Resolution;

void main() {
    gPosition = vec4(FragPos, 1.0);
    
    gNormal = normalize(Normal);

    if (albedoTextureIdx >= 0)
        gAlbedo = vec4(textureLod(MeshTextures, vec3(TexCoords, float(albedoTextureIdx)), 0.0).rgb, 1.0);
    else
        gAlbedo = vec4(Albedo, 1.0);

    if (roughnessTextureIdx >= 0)
        gRoughness = textureLod(MeshTextures, vec3(TexCoords, float(roughnessTextureIdx)), 0.0).r;
    else
        gRoughness = Roughness;

    gIsDielectric = isDielectric ? 1.0 : 0.0;
    gIOR = IOR;

    gIsEmissor = isEmissor ? 1.0 : 0.0;

    vec2 UV = gl_FragCoord.xy / Resolution;
    vec4 PrevClipPos = prevProjection * prevView * vec4(FragPos, 1.0f);
    vec2 prevUV = (PrevClipPos.xy / PrevClipPos.w) * 0.5 + 0.5;

    gMotion = UV - prevUV;
}