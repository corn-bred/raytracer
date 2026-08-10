#pragma once
#include <string>
#include <sstream>
#include "shaders.h"
#include "computeshader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "bvh.h"

class ComputeObjectArray {
    public:

    std::vector<Object> &Objects;

    std::vector<int> LightIndices;

    ComputeObjectArray(std::vector<Object> &objects) : Objects(objects) {}

    void addTriangle(Vertex Vert1, Vertex Vert2, Vertex Vert3, glm::vec3 albedo = glm::vec3(0.0), float roughness = 1, int AlbedoIdx = -1, int RoughnessIdx = -1, float ior = -1.0f) {
        int index = Objects.size();
        Objects.push_back(Object());
        Objects[index].triangle.v0 = Vert1;
        Objects[index].triangle.v1 = Vert2;
        Objects[index].triangle.v2 = Vert3;
        Objects[index].roughness = roughness;
        Objects[index].albedo = albedo;
        Objects[index].emissive = 0;
        Objects[index].albedoTextureIdx = AlbedoIdx;
        Objects[index].roughnessTextureIdx = RoughnessIdx;
        
        if (ior >= 0) {
            Objects[index].dielectric = 1;
            Objects[index].ior = ior;
        } else {
            Objects[index].dielectric = 0;
        }

        Objects[index].padding0[0] = 0.0f;
        Objects[index].padding0[1] = 0.0f;
        Objects[index].padding2[0] = 0.0f;
        Objects[index].padding2[1] = 0.0f;
    }

    void addTriangleEmission(Vertex Vert1, Vertex Vert2, Vertex Vert3, glm::vec3 lightstrength = glm::vec3(4.0), float roughness = 1) {
        int index = Objects.size();
        Objects.push_back(Object());
        Objects[index].triangle.v0 = Vert1;
        Objects[index].triangle.v1 = Vert2;
        Objects[index].triangle.v2 = Vert3;
        Objects[index].roughness = roughness;
        Objects[index].albedo = lightstrength;
        Objects[index].emissive = 1;
        Objects[index].albedoTextureIdx = -1;
        Objects[index].roughnessTextureIdx = -1;
        
        Objects[index].dielectric = 0;

        Objects[index].padding0[0] = 0.0f;
        Objects[index].padding0[1] = 0.0f;
        Objects[index].padding2[0] = 0.0f;
        Objects[index].padding2[1] = 0.0f;

        LightIndices.push_back(index);
    }
};

struct VertexData {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class GBufferManager {
    std::vector<Object> &_Objects;
    Shader &_LinkedShader;
    VertexBuffer *_Data;

    public:
    /*
    layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;*/
    GBufferManager(Shader &designatedShader, std::vector<Object> &objects) : _LinkedShader(designatedShader), _Objects(objects) {
        std::vector<VertexData> vboData;
        vboData.reserve(_Objects.size() * 3);

        for (auto x : _Objects) {
            vboData.push_back({x.triangle.v0.Position, x.triangle.v0.Normal, x.triangle.v0.TexCoords});

            vboData.push_back({x.triangle.v1.Position, x.triangle.v1.Normal, x.triangle.v1.TexCoords});

            vboData.push_back({x.triangle.v2.Position, x.triangle.v2.Normal, x.triangle.v2.TexCoords});
        }

        _Data = new VertexBuffer(vboData.data(), vboData.size() * sizeof(VertexData), GL_STATIC_DRAW);
        _Data->addAttribute(0, 8, GL_FLOAT, 3, 0);
        _Data->addAttribute(1, 8, GL_FLOAT, 3, 3);
        _Data->addAttribute(2, 8, GL_FLOAT, 2, 6);
    }
};

class RasterLightArray {
    public:

    RasterLightArray(Shader &designatedshader, std::string objectarrayname) : _LinkedShader(designatedshader), _ObjectArrayName(objectarrayname) {}

    RasterLightArray(Shader &designatedshader) : _LinkedShader(designatedshader) {}

    RasterLightArray() = delete;

    void modifyObjectName(std::string objectarrayname) {
        _ObjectArrayName = objectarrayname;
    }

    void addLightPoint(unsigned int id, glm::vec3 position, glm::vec3 colour, float constant, float linear, float quadratic) {
        _LinkedShader.setInt(GetUniformName(id, "LightType"), 1);
        _LinkedShader.setVec3(GetUniformName(id, "Position"), position);
        _LinkedShader.setVec3(GetUniformName(id, "Colour"), colour);
        _LinkedShader.setFloat(GetUniformName(id, "Constant"), constant);
        _LinkedShader.setFloat(GetUniformName(id, "Linear"), linear);
        _LinkedShader.setFloat(GetUniformName(id, "Quadratic"), quadratic);
    }

    void addLightSun(unsigned int id, glm::vec3 direction, glm::vec3 colour, float constant, float linear, float quadratic) {
        _LinkedShader.setInt(GetUniformName(id, "LightType"), 2);
        _LinkedShader.setVec3(GetUniformName(id, "Direction"), direction);
        _LinkedShader.setVec3(GetUniformName(id, "Colour"), colour);
        _LinkedShader.setFloat(GetUniformName(id, "Constant"), constant);
        _LinkedShader.setFloat(GetUniformName(id, "Linear"), linear);
        _LinkedShader.setFloat(GetUniformName(id, "Quadratic"), quadratic);
    }
    
    void addLightSpotlight(unsigned int id, glm::vec3 position, glm::vec3 direction, glm::vec3 colour, float innerCutoff, float outerCutoff, float constant, float linear, float quadratic) {
        _LinkedShader.setInt(GetUniformName(id, "LightType"), 3);
        _LinkedShader.setVec3(GetUniformName(id, "Position"), position);
        _LinkedShader.setVec3(GetUniformName(id, "Direction"), direction);
        _LinkedShader.setVec3(GetUniformName(id, "Colour"), colour);
        _LinkedShader.setFloat(GetUniformName(id, "InnerCutoff"), innerCutoff);
        _LinkedShader.setFloat(GetUniformName(id, "OuterCutoff"), outerCutoff);
        _LinkedShader.setFloat(GetUniformName(id, "Constant"), constant);
        _LinkedShader.setFloat(GetUniformName(id, "Linear"), linear);
        _LinkedShader.setFloat(GetUniformName(id, "Quadratic"), quadratic);
    }

    private:

    std::string GetUniformName(int id, std::string name) {
        std::stringstream UniformName;
        UniformName << _ObjectArrayName << "[" << id << "]." << name;
        return UniformName.str();
    }
    Shader &_LinkedShader;
    std::string _ObjectArrayName;
};