#pragma once
#include <string>
#include <sstream>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.h"
#include "computeshader.h"
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

struct MaterialData {
    GLintptr StartData;
    GLsizei DataCount;

    int AlbedoIdx = -1;
    int RoughnessIdx = -1;

    glm::vec3 fallbackAlbedo;
    float fallbackRoughness;

    bool isDielectric;
    float IOR;
};

struct MaterialKey {
    int albedoIdx;
    int roughnessIdx;
    glm::vec3 fallbackAlbedo;
    float fallbackRoughness;
    bool isDielectric;
    float IOR;

    bool operator<(const MaterialKey& other) const { // glm::vec3 does not have a comparison operator
        if (albedoIdx != other.albedoIdx) return albedoIdx < other.albedoIdx;
        
        if (roughnessIdx != other.roughnessIdx) return roughnessIdx < other.roughnessIdx;
        
        if (fallbackAlbedo.x != other.fallbackAlbedo.x) return fallbackAlbedo.x < other.fallbackAlbedo.x;
        if (fallbackAlbedo.y != other.fallbackAlbedo.y) return fallbackAlbedo.y < other.fallbackAlbedo.y;
        if (fallbackAlbedo.z != other.fallbackAlbedo.z) return fallbackAlbedo.z < other.fallbackAlbedo.z;
        
        if (fallbackRoughness != other.fallbackRoughness) return fallbackRoughness < other.fallbackRoughness;

        if (isDielectric != other.isDielectric) return isDielectric < other.isDielectric;
        
        return IOR < other.IOR;
    }
};

class GBufferManager {
    std::vector<Object> _Objects;
    Shader &_LinkedShader;
    VertexBuffer *_Data;
    std::vector<MaterialData> Materials;

    public:
    
    GBufferManager(Shader &designatedShader, std::vector<Object> &objects, bool keepLights = true) : _LinkedShader(designatedShader) {
        if (!keepLights) {
            for (auto &object : objects) {
                if (!object.emissive) _Objects.push_back(object);
            }
        } else {
            _Objects = objects;
        }

        std::map<MaterialKey, std::vector<int>> MaterialGroups;

        for (int i = 0; i < _Objects.size(); i++) {
            int albedoIdx = _Objects[i].albedoTextureIdx;
            int roughnessIdx = _Objects[i].roughnessTextureIdx;
            glm::vec3 fallbackAlbedo = _Objects[i].albedo;
            float fallbackRoughness = _Objects[i].roughness;
            bool isDielectric = _Objects[i].dielectric;
            float IOR = _Objects[i].ior;
            MaterialGroups [ MaterialKey {albedoIdx, roughnessIdx, fallbackAlbedo, fallbackRoughness, isDielectric, IOR} ] .push_back(i);
        }

        std::vector<VertexData> vboData;
        vboData.reserve(_Objects.size() * 3);

        for (const auto &pair : MaterialGroups) {
            int albedoIdx = pair.first.albedoIdx;
            int roughnessIdx = pair.first.roughnessIdx;
            glm::vec3 fallbackAlbedo = pair.first.fallbackAlbedo;
            float fallbackRoughness = pair.first.fallbackRoughness;
            bool isDielectric = pair.first.isDielectric;
            float IOR = pair.first.IOR;

            const std::vector<int> &ObjectIndices = pair.second;

            MaterialData data;
            data.StartData = vboData.size();
            data.AlbedoIdx = albedoIdx;
            data.RoughnessIdx = roughnessIdx;
            data.fallbackAlbedo = fallbackAlbedo;
            data.fallbackRoughness = fallbackRoughness;
            data.isDielectric = isDielectric;
            data.IOR = IOR;

            for (int ObjectIdx : ObjectIndices) {
                auto &x = _Objects[ObjectIdx];
                vboData.push_back({x.triangle.v0.Position, x.triangle.v0.Normal, x.triangle.v0.TexCoords});

                vboData.push_back({x.triangle.v1.Position, x.triangle.v1.Normal, x.triangle.v1.TexCoords});

                vboData.push_back({x.triangle.v2.Position, x.triangle.v2.Normal, x.triangle.v2.TexCoords});
            }

            data.DataCount = vboData.size() - data.StartData;
            Materials.push_back(data);
        }

        _Data = new VertexBuffer(vboData.data(), vboData.size() * sizeof(VertexData), GL_STATIC_DRAW);
        _Data->addAttribute(0, 3, GL_FLOAT, 8, 0);
        _Data->addAttribute(1, 3, GL_FLOAT, 8, 3);
        _Data->addAttribute(2, 2, GL_FLOAT, 8, 6);

        _LinkedShader.use();
        _Data->bind();
    }

    void draw() {
        _Data->bind();

        glEnable(GL_DEPTH_TEST);

        for (const auto &material : Materials) {
            _LinkedShader.setInt("albedoTextureIdx", material.AlbedoIdx);
            _LinkedShader.setInt("roughnessTextureIdx", material.RoughnessIdx);
            _LinkedShader.setVec3("Albedo", material.fallbackAlbedo);
            _LinkedShader.setFloat("Roughness", material.fallbackRoughness);
            _LinkedShader.setBool("isDielectric", material.isDielectric);
            _LinkedShader.setFloat("IOR", material.IOR);
                
            glDrawArrays(GL_TRIANGLES, material.StartData, material.DataCount);
        }

        _Data->unbind();
    }

    void bindTextures(GLuint textureArrayID) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
        _LinkedShader.setInt("MeshTextures", 1);
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