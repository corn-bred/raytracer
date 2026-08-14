#version 430 core

out vec4 FragColor;

uniform sampler2D Raster;
uniform sampler2D Raytrace;
in vec2 TexCoords;


float linearToGamma(float LinearComponent)
{
    if (LinearComponent > 0)
        return pow(LinearComponent, 1.0 / 2.2);
    
    return 0;
}

void main () {
    vec3 RasterData = texture(Raster, TexCoords).rgb;
    vec3 RaytraceData = texture(Raytrace, TexCoords).rgb;
    vec3 Result = RaytraceData;
    Result = vec3(linearToGamma(Result.r), linearToGamma(Result.g), linearToGamma(Result.b));
    FragColor = vec4(Result, 1.0);
}