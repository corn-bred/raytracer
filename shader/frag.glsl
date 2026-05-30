#version 420 core

in vec2 TexCoords;

out vec4 FragColor;

uniform mat4 invProjection;
uniform mat4 View;
uniform vec4 CameraPos;

class Ray {
    vec3 orig;
    vec3 dir;

    vec3 At(double t) {
        return orig + dir * vec3(t);
    }
};

void main () {
    vec4 NDCoords = vec4(TexCoords.xy * 2 - 1.0, -1.0, 1.0);

    vec4 Result = NDCoords * invProjection;
    Result = vec4(Result.xyz / Result.w, 1.0);

    Result = Result * View;
    Result = vec4(Result.xyz / Result.w, 1.0);
    Result = normalize(Result);
    Ray ray;
    ray.orig = vec3(CameraPos.xyz);
    ray.dir = vec3(Result.xyz);

    FragColor = vec4(ray.dir, 1.0);
}