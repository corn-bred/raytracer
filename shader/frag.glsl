#version 420 core

#define HITTABLE_LIST_ARRAY_SIZE 4

in vec2 TexCoords;

out vec4 FragColor;

uniform mat4 invProjection;
uniform mat4 invView;
uniform vec3 CameraPos;
uniform float glfwTime;
uniform int Width;
uniform int Height;
uniform int MSAAsamples;
uniform int MaximumDepth;
uniform sampler2D TextureAccumulation;
uniform int FrameIndex;

const float infinity = 1.0 / 0.0;
const double pi = 3.1415926535897932385;

double degreesToRadians(double degrees) {
    return degrees * pi / 180.0;
}

double rand(double seed) {
    return fract(sin(float(seed)) * 43758.5453123);
}

double randRange(double seed, double min, double max) {
    return min + (max - min) * rand(seed);
}

vec3 sampleSquare(float seed, int sampleIndex) {
    float seedX = seed + float(sampleIndex) + TexCoords.x + glfwTime;
    float seedY = seedX + 3.12534 + TexCoords.y;
    return vec3(randRange(seedX, -0.5, 0.5), randRange(seedY, -0.5, 0.5), 0.0);
}

vec3 randVec3(double seed) {
    return vec3(rand(seed), rand(seed + 10.3454), rand(seed + 5.3543));
}

vec3 randRangeVec3(double seed, double min, double max) {
    return vec3(randRange(seed, min, max), randRange(seed + 10.3454, min, max), randRange(seed + 5.3543, min, max));
}

vec3 randUnitVec3(double seed){
    for (int i = 0; i < 50; i++) {
        vec3 p = randRangeVec3(gl_FragCoord.x * 1.243 + gl_FragCoord.y * 6.23584 + i * 1.43584 + seed, -1, 1);
        float LengthSquared = length(p)*length(p); //Length squared, why is it named that
        if ( 1e-8 < LengthSquared && LengthSquared <= 1) { // Uh oh, underflow protector
            return normalize(p);
        }
    }
    return vec3(1.0, 0.0, 0.0);
}

vec3 randOnHemisphere(double seed, vec3 Normal) {
    vec3 OnUnitSphere = randUnitVec3(seed);
    if(dot(OnUnitSphere, Normal) > 0.0) {
        return OnUnitSphere;
    } else {
        return -OnUnitSphere;
    }
}

float reflectance(float cosine, float ior) {
    float r0 = (1 - ior) / (1 + ior);
    r0 = pow(r0, 2);
    return r0 + (1-r0) * pow((1 - cosine), 5);
}

//      Ray

struct Ray {
    vec3 Origin;
    vec3 Direction;
};

vec3 RayAt(inout Ray ray, double t) {
    return ray.Origin + ray.Direction * vec3(t);
}

//      HitRecord

struct HitRecord {
    vec3 Position;
    vec3 Normal;
    float t;
    bool facingFront;
    float Roughness;
    vec3 Albedo;
    bool isDielectric;
    float ior;

    //void HitRecordSetFaceNormal();
};

void HitRecordSetFaceNormal(inout HitRecord hitrecord, Ray r, vec3 outwardNormal) {
    hitrecord.facingFront = (dot(r.Direction, outwardNormal) < 0.0);
    hitrecord.Normal = hitrecord.facingFront? outwardNormal : -outwardNormal;
}



//      Sphere

struct Sphere {
    vec3 center;
    float radius;
    float roughness;
    vec3 albedo;
    bool dielectric;
    float ior;

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

    //copy data
    rec.Roughness = sphere.roughness;
    rec.Albedo = sphere.albedo;
    rec.isDielectric = sphere.dielectric;
    rec.ior = sphere.ior;

    vec3 outwardNormal = vec3(rec.Position - sphere.center) / vec3(sphere.radius);
    HitRecordSetFaceNormal(rec, ray, outwardNormal);

    return true;
}

uniform Sphere uObjects[HITTABLE_LIST_ARRAY_SIZE];



//      HittableList

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




Ray getRay(float i, float j, int SampleNumber) {
    vec3 Offset = sampleSquare(2, SampleNumber);
    vec2 PixelSample = vec2(i + Offset.x + 0.5, j + Offset.y + 0.5);
    vec4 NDCoords = vec4(PixelSample.x / Width * 2.0 - 1.0, (PixelSample.y / Height * 2.0 - 1.0), -1.0, 1.0);

    vec3 RayOrigin = CameraPos;
    vec4 RayDirection = invProjection * NDCoords;
    RayDirection = vec4(RayDirection.xyz / RayDirection.w, 1.0);
    RayDirection = normalize(invView * vec4(RayDirection.xyz, 0.0));

    Ray r;
    r.Origin = RayOrigin;
    r.Direction = vec3(RayDirection.xyz);

    return r;
}





vec3 rayColour(Ray r, int maxDepth, HittableList world) {
    vec3 colour = vec3(1.0);

    HitRecord rec;
    if (HittableListHitSphere(world, r, 0.001, infinity, rec)) {
        if (!rec.isDielectric)
        colour = rec.Albedo * colour;

        for (int depth = 0; depth < maxDepth; ++depth) {
            vec3 dir = vec3(0.0);

            if (rec.isDielectric) {
                float ri = rec.facingFront ? (1.0/rec.ior) : rec.ior;
                vec3 Incident = normalize(r.Direction);
                vec3 Normal = normalize(rec.Normal);
                vec3 Refracted = refract(Incident, Normal, ri);

                float CosTheta = min(dot(-Incident, Normal), 1.0);

                if (Refracted == vec3(0.0) || reflectance(CosTheta, ri) > rand(double(glfwTime + gl_FragCoord.x * 3.36 + gl_FragCoord.y * 1.53 + depth * 1.3454))) {
                    dir = reflect(Incident, Normal);
                } else {
                    dir = Refracted;
                }
            } else {
                dir = normalize(reflect(r.Direction, rec.Normal) + randOnHemisphere(gl_FragCoord.x + gl_FragCoord.y + glfwTime + float(depth), rec.Normal)* rec.Roughness);
            }

            
            r.Origin = rec.Position;
            r.Direction = dir;

            if(HittableListHitSphere(world, r, 0.001, infinity, rec)) {
                if (!rec.isDielectric)
                colour = rec.Albedo * colour;
            } else {
                vec3 unitDir = normalize(r.Direction);
                float a = 0.5 * (unitDir.y + 1.0);
                vec3 sky = (1.0 - a) * vec3(1.0) + a * vec3(0.5, 0.7, 1.0);
                colour *= sky;
                return colour;
            }

        }

    } else {

        vec3 unitDir = normalize(r.Direction);
        float a = 0.5 * (unitDir.y + 1.0);
        vec3 sky = (1.0 - a) * vec3(1.0) + a * vec3(0.5, 0.7, 1.0);
        colour *= sky;

    }

    return colour;
}




void main () {
    HittableList world;
    world.objects = uObjects;

    vec3 PreviousSample = texture(TextureAccumulation, TexCoords).rgb;

    vec3 PixelColour = vec3(0.0);
    for(int samples = 0; samples < MSAAsamples; samples++) {
        Ray r = getRay(gl_FragCoord.x, gl_FragCoord.y, samples);
        PixelColour += rayColour(r, MaximumDepth, world);
    }
    PixelColour /= float(MSAAsamples);

    vec3 NewSample = PixelColour;

    float TotalSamples = float(FrameIndex + 1);
    vec3 AccumulatedColour = (PreviousSample * float(FrameIndex) + NewSample) / TotalSamples;

    if (FrameIndex == 1) {
        AccumulatedColour = NewSample;
    }
    FragColor = vec4(AccumulatedColour, 1.0);
}