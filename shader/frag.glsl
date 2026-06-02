#version 420 core

in vec2 TexCoords;

out vec4 FragColor;

uniform mat4 invProjection;
uniform mat4 invView;
uniform vec3 CameraPos;

class HitRecord {
    vec3 Position;
    vec3 Normal;
    float t;
}

class Ray {
    vec3 Origin;
    vec3 Direction;

    vec3 At(double t) {
        return Origin + Direction * vec3(t);
    }
};

class Sphere {
    Sphere(const point3& center, double radius) : center(center), radius(fmax(0,radius)) {}

    bool hit(Ray r, double ray_tmin, double ray_tmax, inout HitRecord rec) {
        vec3 OriginToCenter = center - ray.Origin;
        // Q u a d r a t i c
        double a = pow(length(ray.Direction), 2); // Should be 1
        double h = dot(ray.Direction, OriginToCenter); // dir*oc
        double c = pow(length(OriginToCenter), 2) - radius * radius; // length(oc)^2 - r^2
        double discriminant = h*h - a*c;

        if (discriminant < 0)
            return false;

        double sqrtd = sqrt(discriminant);

        // Find nearest root
        double root = (h - sqrtd) / a;
        if (root <= ray_tmin || ray_tmax <= root) {
            root = (h + sqrtd) / a;
            if (root <= ray_tmin || ray_tmax <= root)
                return false;
        }

        rec.t = root;
        rec.Position = r.At(rec.t);
        rec.Normal = (rec.Position - center) / radius;

        return true;
    }

    vec3 center;
    double radius;
}

vec3 rayColour(Ray r) {
    float t = willIntersectSphere(vec3(0,0,-1), 0.5, r);

    if (t > 0.0) {
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