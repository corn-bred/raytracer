#version 420 core

#define HITTABLE_LIST_ARRAY_SIZE 2

in vec2 TexCoords;

out vec4 FragColor;

uniform mat4 invProjection;
uniform mat4 invView;
uniform vec3 CameraPos;

const double infinity = 1.0 / 0.0;
const double pi = 3.1415926535897932385;

struct Ray {
    vec3 Origin;
    vec3 Direction;
};

vec3 RayAt(inout Ray ray, double t) {
    return ray.Origin + ray.Direction * vec3(t);
}

struct HitRecord {
    vec3 Position;
    vec3 Normal;
    float t;
    bool facingFront;

    //void HitRecordSetFaceNormal();
};

void HitRecordSetFaceNormal(inout HitRecord hitrecord, Ray r, vec3 outwardNormal) {
    hitrecord.facingFront = (dot(r.Direction, outwardNormal) < 0.0);
    hitrecord.Normal = hitrecord.facingFront? outwardNormal : -outwardNormal;
}

struct Sphere {
    vec3 center;
    float radius;

    //bool hit();
};

// spagetti code go
bool SphereHit(inout Sphere sphere, Ray ray, double ray_tmin, double ray_tmax, inout HitRecord rec) {
    vec3 OriginToCenter = sphere.center - ray.Origin;
    // Q u a d r a t i c
    double a = pow(length(ray.Direction), 2); // Should be 1
    double h = dot(ray.Direction, OriginToCenter); // dir*oc
    double c = pow(length(OriginToCenter), 2) - sphere.radius * sphere.radius; // length(oc)^2 - r^2
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

    rec.t = float(root);
    rec.Position = RayAt(ray, rec.t);
    vec3 outwardNormal = vec3(rec.Position - sphere.center) / vec3(sphere.radius);
    HitRecordSetFaceNormal(rec, ray, outwardNormal);

    return true;
}

uniform Sphere uObjects[HITTABLE_LIST_ARRAY_SIZE];

struct HittableList {
    Sphere objects[HITTABLE_LIST_ARRAY_SIZE];
    //bool hitSphere();
};

bool HittableListHitSphere(inout HittableList hittablelist, Ray r, double ray_tmin, double ray_tmax, inout HitRecord rec) {
    HitRecord TempRec;
    bool anythingHit = false;
    double ClosestSoFar = ray_tmax;

    for (int i = 0; i < HITTABLE_LIST_ARRAY_SIZE; i++) {
        Sphere object = hittablelist.objects[i];
        if(SphereHit(object, r, ray_tmin, ClosestSoFar, TempRec)) {
            anythingHit = true;
            ClosestSoFar = TempRec.t;
            rec = TempRec;
        }
    }

    return anythingHit;
}

vec3 rayColour(Ray r, HittableList world) {
    HitRecord rec;

    if(HittableListHitSphere(world, r, 0, infinity, rec)) {
        return 0.5 * (rec.Normal + vec3(1.0, 1.0, 1.0));
    }

    vec3 UnitDirection = normalize(r.Direction);
    
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

    HittableList world;
    world.objects = uObjects;

    FragColor = vec4(rayColour(ray, world), 1.0);
}