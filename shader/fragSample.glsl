#version 330 core

out vec4 FragColor;

uniform sampler2D TextureAccumulation;
in vec2 TexCoords;


float linearToGamma(float LinearComponent)
{
    if (LinearComponent > 0)
        return sqrt(LinearComponent);
    
    return 0;
}

void main () {
    vec3 TextureData = texture(TextureAccumulation,TexCoords).rgb;
    vec3 Result = vec3(linearToGamma(TextureData.r), linearToGamma(TextureData.g), linearToGamma(TextureData.b));
    FragColor = vec4(Result, 1.0);
}