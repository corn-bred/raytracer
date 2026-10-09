#version 430 core

out vec4 FragColor;

uniform sampler2D Raster;
uniform sampler2D Raytrace;
in vec2 TexCoords;

uniform int OutputType;

#define OUTPUT_RASTER 0
#define OUTPUT_RAYTRACE 1
#define OUTPUT_COMBINED 2
#define OUTPUT_COMPARISON 3


float linearToGamma(float LinearComponent)
{
    if (LinearComponent > 0)
        return pow(LinearComponent, 1.0 / 2.2);
    
    return 0;
}

void main () {
    vec3 RasterData = texture(Raster, TexCoords).rgb;
    vec3 RaytraceData = texture(Raytrace, TexCoords).rgb;
    vec3 Result;

    switch (OutputType) {
        case OUTPUT_RASTER:
            Result = RasterData;
            break;
        case OUTPUT_RAYTRACE:
            Result = RaytraceData;
            break;
        case OUTPUT_COMBINED:
            Result = RasterData + RaytraceData;
            break;
        case OUTPUT_COMPARISON:
            if (TexCoords.x < 0.3) {
                Result = RasterData;
            } else if (TexCoords.x < 0.7) {
                Result = RaytraceData + RasterData; 
            } else {
                Result = RaytraceData;
            }
            break;
        default:
            break;
    }

    Result = vec3(linearToGamma(Result.r), linearToGamma(Result.g), linearToGamma(Result.b));
    FragColor = vec4(Result, 1.0);
}