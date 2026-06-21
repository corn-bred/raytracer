#version 330 core

#define HITTABLE_LIST_ARRAY_SIZE 14

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
uniform float DefocusAngle;
uniform float FocusDist;

const float infinity = 1.0 / 0.0;
const float pi = 3.1415926535897932385;
float DefocusRadius;

float degreesToRadians(float degrees) {
    return degrees * pi / 180.0;
}

float rand(float seed) {
    return fract(sin(float(seed * 12.9898)) * 43758.5453123);
}

float randRange(float seed, float min, float max) {
    return min + (max - min) * rand(seed);
}

vec3 sampleSquare(float seed, int sampleIndex) {
    float seedX = seed + float(sampleIndex) + TexCoords.x + glfwTime;
    float seedY = seedX + 3.12534 + TexCoords.y;
    return vec3(randRange(seedX, -0.5, 0.5), randRange(seedY, -0.5, 0.5), 0.0);
}

vec3 randVec3(float seed) {
    return vec3(rand(seed), rand(seed + 10.3454), rand(seed + 5.3543));
}

vec3 randRangeVec3(float seed, float min, float max) {
    return vec3(randRange(seed, min, max), randRange(seed + 10.3454, min, max), randRange(seed + 5.3543, min, max));
}

vec3 randUnitVec3(float seed){
    float theta = float(randRange(seed, 0.0, 2.0 * pi));
    float z = float(randRange(seed + 1.0, -1.0, 1.0)); //r depends on z
    float r = sqrt(1.0 - z*z);
    return vec3(r * cos(theta), r * sin(theta), z);
}

vec2 randRangeVec2(float seed, float min, float max) {
    return vec2(randRange(seed, min, max), randRange(seed + 10, min, max));
}

vec2 randUnitVec2(float seed){
    float r = float(sqrt(randRange(seed, 0.0, 1.0)));
    float theta = float(randRange(seed + 0.5, 0.0, 2.0 * pi));
    return vec2(r * cos(theta), r * sin(theta));
}

vec3 randOnHemisphere(float seed, vec3 Normal) {
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

vec3 RayAt(inout Ray ray, float t) {
    return ray.Origin + ray.Direction * vec3(t);
}

//      HitRecord

struct HitRecord {
    vec3 Position;

    vec3 TriPoint1;
    vec3 TriPoint2;
    vec3 TriPoint3;

    vec3 Normal;
    float t;
    bool facingFront;
    float Roughness;
    vec3 Albedo;

    bool isDielectric;
    float ior;

    bool isEmissive;

    //void HitRecordSetFaceNormal();
};

void HitRecordSetFaceNormal(inout HitRecord hitrecord, Ray r, vec3 outwardNormal) {
    hitrecord.facingFront = (dot(r.Direction, outwardNormal) < 0.0);
    hitrecord.Normal = hitrecord.facingFront? outwardNormal : -outwardNormal;
}



//      Sphere

struct Object {
    int objecttype; //0: Sphere, 1: Triangle
    vec3 Spherecenter;
    float Sphereradius;

    vec3 Trianglep1;
    vec3 Trianglep2;
    vec3 Trianglep3;

    float roughness;
    vec3 albedo;

    bool dielectric;
    float ior;

    bool emissive;

    //bool hit();
};

// spagetti code go
bool SphereHit(inout Object object, Ray ray, float ray_tmin, float ray_tmax, inout HitRecord rec) {
    vec3 OriginToCenter = object.Spherecenter - ray.Origin;
    // Q u a d r a t i c
    float a = pow(length(ray.Direction), 2); // Should be 1
    float h = dot(ray.Direction, OriginToCenter); // dir*oc
    float c = pow(length(OriginToCenter), 2) - object.Sphereradius * object.Sphereradius; // length(oc)^2 - r^2
    float discriminant = h*h - a*c;

    if (discriminant < 0)
        return false;

    float sqrtd = sqrt(discriminant);

    // Find nearest root
    float root = (h - sqrtd) / a;
    if (root <= ray_tmin || ray_tmax <= root) {
        root = (h + sqrtd) / a;
        if (root <= ray_tmin || ray_tmax <= root)
            return false;
    }

    rec.t = float(root);
    rec.Position = RayAt(ray, rec.t);

    //copy data
    rec.Roughness = object.roughness;
    rec.Albedo = object.albedo;
    rec.isDielectric = object.dielectric;
    rec.ior = object.ior;
    rec.isEmissive = object.emissive;

    vec3 outwardNormal = vec3(rec.Position - object.Spherecenter) / vec3(object.Sphereradius);
    HitRecordSetFaceNormal(rec, ray, outwardNormal);

    return true;
}

bool TriangleHit(inout Object object, Ray ray, float ray_tmin, float ray_tmax, inout HitRecord rec) {
    vec3 Edge1 = object.Trianglep2 - object.Trianglep1;
    vec3 Edge2 = object.Trianglep3 - object.Trianglep1;

    vec3 tVec = ray.Origin - object.Trianglep1; // Arrow from point 1 to ray origin
    vec3 pVec = cross(ray.Direction, Edge2);    // Perpendicular arrow of the ray direction
    vec3 qVec = cross(tVec, Edge1);             // 

    float det = dot(Edge1, pVec);
    if (det > -0.001 && det < 0.001) return false;
    float invdet = 1.0 / det;

    float u = dot(tVec, pVec) * invdet;
    float v = dot(ray.Direction, qVec) * invdet;

    if (!((u >= 0 && u <= 1) && (v >= 0 && v <= 1) && (u + v) < 1)) return false;

    float t = dot(Edge2, qVec) * invdet;

    if (t < ray_tmin || t > ray_tmax) return false;

    if (t < 0) return false;

    rec.t = t;

    vec3 normal = normalize(cross(Edge1, Edge2));

    rec.Position = RayAt(ray, rec.t);

    if (det < 0) normal *= -1;

    rec.Normal = normal;

    rec.TriPoint1 = object.Trianglep1;
    rec.TriPoint2 = object.Trianglep2;
    rec.TriPoint3 = object.Trianglep3;

    rec.Roughness = object.roughness;
    rec.Albedo = object.albedo;
    rec.isDielectric = object.dielectric;
    rec.ior = object.ior;
    rec.isEmissive = object.emissive;

    return true;
}

uniform Object uObjects[HITTABLE_LIST_ARRAY_SIZE];



//      HittableList

struct HittableList {
    Object objects[HITTABLE_LIST_ARRAY_SIZE];
    //bool hitSphere();
};

bool HittableListHit(inout HittableList hittablelist, Ray r, float ray_tmin, float ray_tmax, inout HitRecord rec) {
    HitRecord TempRec;
    bool anythingHit = false;
    float ClosestSoFar = ray_tmax;

    for (int i = 0; i < HITTABLE_LIST_ARRAY_SIZE; i++) {
        Object object = hittablelist.objects[i];
        if(object.objecttype == 0) {
            if(SphereHit(object, r, ray_tmin, ClosestSoFar, TempRec)) {
                anythingHit = true;
                ClosestSoFar = TempRec.t;
                rec = TempRec;
            }
        } else if (object.objecttype == 1) {
            if(TriangleHit(object, r, ray_tmin, ClosestSoFar, TempRec)) {
                anythingHit = true;
                ClosestSoFar = TempRec.t;
                rec = TempRec;
            }
        }
    }

    return anythingHit;
}




Ray getRay(float i, float j, int SampleNumber) {
    vec3 Offset = sampleSquare(2, SampleNumber);
    vec2 PixelSample = vec2(i + Offset.x + 0.5, j + Offset.y + 0.5);

    //Normalization
    vec4 NDCoords = vec4(PixelSample.x / Width * 2.0 - 1.0, (PixelSample.y / Height * 2.0 - 1.0), -1.0, 1.0);

    //Finding Camera Position
    vec4 CameraTemp = invProjection * NDCoords; //vec4 CameraTemp is a temporary for vec3 PinholeDir as the final variable.
    CameraTemp = vec4(CameraTemp.xyz / CameraTemp.w, 1.0);
    vec3 PinholeDir = normalize((invView * vec4(CameraTemp.xyz, 0.0)).xyz);
    
    //Finding focus point in world space
    vec3 focusPoint = CameraPos + PinholeDir * FocusDist;

    //Finding camera unit axes (Extracting them via inverted view matrix)
    vec3 CamRight   = normalize((invView * vec4(1.0, 0.0, 0.0, 0.0)).xyz);
    vec3 CamUp      = normalize((invView * vec4(0.0, 1.0, 0.0, 0.0)).xyz);
    vec3 CamForward = normalize((invView * vec4(0.0, 0.0, -1.0, 0.0)).xyz);

    //Random 2D point on unit (circle) disk
    vec2 DiskPoint = vec2(randUnitVec2(glfwTime + gl_FragCoord.x + gl_FragCoord.y * 2.435 + float(SampleNumber)));

    //Random point onto 3D world space
    vec3 LensOffset = (CamRight * DiskPoint.x + CamUp * DiskPoint.y) * DefocusRadius;

    //Final calculations for ray
    vec3 RayOrigin = CameraPos + LensOffset;
    vec3 RayDirection = normalize(focusPoint - RayOrigin); //Points towards focusPoint in local translation space

    Ray r;
    r.Origin = RayOrigin;
    r.Direction = RayDirection;

    return r;
}





vec3 rayColour(Ray r, int maxDepth, HittableList world) {
    vec3 colour = vec3(1.0);

    HitRecord rec;

    if (HittableListHit(world, r, 0.001, infinity, rec)) {
        if (!rec.isDielectric)
            colour = rec.Albedo * colour;

        for (int depth = 0; depth < maxDepth; ++depth) {
            if (rec.isEmissive) {
                return colour;
            }
            float seed = glfwTime + gl_FragCoord.x * fract(3.96 + glfwTime) + gl_FragCoord.y * fract(5.9 + glfwTime * 2) + depth * fract(2.47 + glfwTime * 3);

            vec3 dir = vec3(0.0);

            vec3 Incident = normalize(r.Direction);
            vec3 Normal = normalize(rec.Normal + randOnHemisphere(seed + 6.35, rec.Normal) * rec.Roughness);

            if (rec.isDielectric) {
                float ri = rec.facingFront ? (1.0/rec.ior) : rec.ior;
                
                vec3 Refracted = refract(Incident, Normal, ri);

                float CosTheta = min(dot(-Incident, Normal), 1.0);

                if (Refracted == vec3(0.0) || reflectance(CosTheta, ri) > rand(seed + 3.92)) {
                    dir = reflect(Incident, Normal);
                } else {
                    dir = Refracted;
                }
            } else {
                dir = reflect(Incident, Normal);
            }

            r.Origin = rec.Position;
            r.Direction = dir;
            float AngFalloff = max(0.0, dot(Normal, -Incident));
            vec3 Attenuation = rec.Albedo * AngFalloff;

            if(HittableListHit(world, r, 0.001, infinity, rec)) {
                if (!rec.isDielectric) {
                    colour = Attenuation * colour;
                    
                }
            } else {
                vec3 sky = vec3(0.0);
                colour *= sky;
                return colour;
            }

        }

    } else {
        vec3 sky = vec3(0.0);
        colour *= sky;
    }

    return colour;
}




void main () {
    HittableList world;
    world.objects = uObjects;

    DefocusRadius = FocusDist * tan(float(degreesToRadians(DefocusAngle / 2)));

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