#version 430 core

in vec2 TexCoords;

out vec4 FragColor;

//G-buffers from deffered rendering

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gRoughness;

uniform vec3 viewPos;

//starts at 1 because uninitialized data will be 0 (if uniform only though)

#define LIGHT_POINT 1
#define LIGHT_SUN 2
#define LIGHT_SPOTLIGHT 3

struct LightUniversal { //not for area lights
    int LightType;

    vec3 Position; // Point & Spotlight

    vec3 Direction; // Sun & Spotlight

    float InnerCutoff; // Spotlight only
    float OuterCutoff; // Spotlight only
  
    vec3 Colour;

    float Constant;
    float Linear;
    float Quadratic;
};

vec3 calculateLightPoint(LightUniversal light, vec3 albedo, float roughness, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 calculateLightSun(LightUniversal light, vec3 albedo, float roughness, vec3 normal, vec3 viewDir);

vec3 calculateLightSpotlight(LightUniversal light, vec3 albedo, float roughness, vec3 normal, vec3 fragPos, vec3 viewDir);

#define NR_LIGHTS 1

uniform LightUniversal pointLights[NR_LIGHTS];

void main() {
    vec3 FragPos = texture(gPosition, TexCoords).xyz;

    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    vec3 Normal = texture(gNormal, TexCoords).rgb;

    float Roughness = texture(gRoughness, TexCoords).r;

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 Result = vec3(0.0);
    for (int i = 0 ; i < NR_LIGHTS; i++) {
        switch(pointLights[i].LightType) {
            case LIGHT_POINT:
                Result += calculateLightPoint(pointLights[i], Albedo, Roughness, Normal, FragPos, viewDir);
                break;
            case LIGHT_SUN:
                Result += calculateLightSun(pointLights[i], Albedo, Roughness, Normal, viewDir);
                break;
            case LIGHT_SPOTLIGHT:
                Result += calculateLightSpotlight(pointLights[i], Albedo, Roughness, Normal, FragPos, viewDir);
                break;
            default:
                break;
        }
    }

    FragColor = vec4(Result, 1.0);
}

vec3 calculateLightPoint(LightUniversal light, vec3 albedo, float roughness, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (light.LightType != LIGHT_POINT) return vec3(0.0);
    vec3 lightDir = normalize(light.Position - fragPos); 

    float dist = length(light.Position - fragPos);
    float attenuation = 1.0f / (light.Constant + light.Linear * dist + light.Quadratic * (dist*dist) );

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * (light.Colour * albedo);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0 - roughness);
    vec3 specular = (1.0 - roughness) * spec * light.Colour;  
    
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = (diffuse + specular);
    return result;
}

vec3 calculateLightSun(LightUniversal light, vec3 albedo, float roughness, vec3 normal, vec3 viewDir) {
    if (light.LightType != LIGHT_SUN) return vec3(0.0);
    vec3 lightDir = normalize(-light.Direction);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * (light.Colour * albedo.rgb);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0 - roughness);
    vec3 specular = (1.0 - roughness) * spec * light.Colour;

    vec3 result = (diffuse + specular);
    return result;
}

vec3 calculateLightSpotlight(LightUniversal light, vec3 albedo, float roughness, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (light.LightType != LIGHT_SPOTLIGHT) return vec3(0.0);

    vec3 surfToLight = normalize(light.Position - fragPos);
    vec3 lightToSurf = normalize(fragPos - light.Position);

    float dist = length(light.Position - fragPos);
    float attenuation = 1.0f / (light.Constant + light.Linear * dist + light.Quadratic * (dist*dist) );

    float diff = max(dot(normal, surfToLight), 0.0);
    vec3 diffuse = diff * (light.Colour * albedo);

    vec3 reflectDir = reflect(-surfToLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0 - roughness);
    vec3 specular = (1.0 - roughness) * spec * light.Colour;  

    //ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float theta = dot(-lightToSurf, normalize(-light.Direction));
    float epsilon = light.InnerCutoff - light.OuterCutoff;
    float intensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;

    vec3 result = (diffuse + specular);
    return result;
}