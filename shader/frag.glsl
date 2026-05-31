#version 420 core

in vec2 TexCoords;

out vec4 FragColor;

uniform mat4 invProjection;
uniform mat4 invView;
uniform vec3 CameraPos;

class Ray {
    vec3 Origin;
    vec3 Direction;

    vec3 At(double t) {
        return Origin + Direction * vec3(t);
    }
};

float willIntersectSphere(vec3 center, double radius, Ray ray) {
    vec3 OriginToCenter = center - ray.Origin;
    // Q u a d r a t i c
    double a = dot(ray.Direction, ray.Direction); // Should be 1
    double b = -2.0 * dot(ray.Direction, OriginToCenter); // 2(dir*oc)
    double c = dot(OriginToCenter, OriginToCenter) - radius * radius; // oc*oc - r^2
    double discriminant = b*b - 4*(a*c);
    if (discriminant < 0) {
        return -1.0;
    } else {
        return float(-b - sqrt(discriminant) ) / float(2.0*a);
    }
}

vec3 rayColour(Ray r) {
    float t = willIntersectSphere(vec3(0,0,-1), 0.5, r);

    if (t != -1.0) {
        vec3 Normal = normalize(r.At(t) -  vec3(0.0, 0.0, -1.0));
        return (0.5 * vec3(Normal.x +1.0, Normal.y +1.0, Normal.z +1.0));
    }
    
    float a = 0.5 * (r.Direction.y + 1.0);
    return ( (1.0-a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5, 0.7, 1.0) );
}

void main () {
    vec4 NDCoords = vec4(TexCoords.xy * 2 - 1.0, -1.0, 1.0);

    vec4 Result = invProjection * NDCoords;
    Result = vec4(Result.xyz / Result.w, 1.0);

    Result = normalize(invView * vec4(Result.xyz, 0.0));

    Ray ray;
    ray.Origin = CameraPos;
    ray.Direction = vec3(Result.xyz);

    FragColor = vec4(rayColour(ray), 1.0);
}