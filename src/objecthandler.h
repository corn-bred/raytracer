#pragma once
#include <string>
#include <sstream>
#include "shaders.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ObjectArray {
    public:

    ObjectArray(Shader &designatedshader, std::string objectarrayname) : _LinkedShader(designatedshader), _ObjectArrayName(objectarrayname) {}

    ObjectArray(Shader &designatedshader) : _LinkedShader(designatedshader) {}

    ObjectArray() = delete;

    void modifyObjectName(std::string objectarrayname) {
        _ObjectArrayName = objectarrayname;
    }

    void setMaterial(unsigned int id, glm::vec3 center = glm::vec3(0.0), float radius = 1, glm::vec3 albedo = glm::vec3(0.0), float roughness = 1, float ior = -1.0f, float specularprob = -1.0f) {
        {
            std::stringstream UniformName;
            UniformName << _ObjectArrayName << "[" << id << "].center";
            _LinkedShader.setVec3(UniformName.str(), center);
        }
        {
            std::stringstream UniformName(_ObjectArrayName);
            UniformName << _ObjectArrayName << "[" << id << "].radius";
            _LinkedShader.setFloat(UniformName.str(), radius);
        }
        {
            std::stringstream UniformName(_ObjectArrayName);
            UniformName << _ObjectArrayName << "[" << id << "].roughness";
            _LinkedShader.setFloat(UniformName.str(), roughness);
        }
        {
            std::stringstream UniformName(_ObjectArrayName);
            UniformName << _ObjectArrayName << "[" << id << "].roughness";
            _LinkedShader.setFloat(UniformName.str(), roughness);
        }
        {
            std::stringstream UniformName(_ObjectArrayName);
            UniformName << _ObjectArrayName << "[" << id << "].albedo";
            _LinkedShader.setVec3(UniformName.str(), albedo);
        }
        if (ior >= 0) {
            {
            std::stringstream UniformName(_ObjectArrayName);
            UniformName << _ObjectArrayName << "[" << id << "].dielectric";
            _LinkedShader.setBool(UniformName.str(), true);
            }
            {
            std::stringstream UniformName(_ObjectArrayName);
            UniformName << _ObjectArrayName << "[" << id << "].ior";
            _LinkedShader.setFloat(UniformName.str(), ior);
            }
        }
        if (ior >= 0) {
            {
            std::stringstream UniformName(_ObjectArrayName);
            UniformName << _ObjectArrayName << "[" << id << "].thinlayer";
            _LinkedShader.setBool(UniformName.str(), true);
            }
            {
            std::stringstream UniformName(_ObjectArrayName);
            UniformName << _ObjectArrayName << "[" << id << "].specularprob";
            _LinkedShader.setFloat(UniformName.str(), specularprob);
            }
        } 
    }

    /*vec3 center;
    float radius;
    float roughness;
    vec3 albedo;
    bool dielectric;
    float ior;*/
    private:
    Shader &_LinkedShader;
    std::string _ObjectArrayName;
};