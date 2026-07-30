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

class ObjectArray {
    public:

    std::vector<Object> &Objects;

    ObjectArray(std::vector<Object> &objects) : Objects(objects) {}

    void addTriangle(Vertex Vert1, Vertex Vert2, Vertex Vert3, glm::vec3 albedo = glm::vec3(0.0), float roughness = 1, float ior = -1.0f) {
        int index = Objects.size();
        Objects.push_back(Object());
        Objects[index].triangle.v0 = Vert1;
        Objects[index].triangle.v1 = Vert2;
        Objects[index].triangle.v2 = Vert3;
        Objects[index].roughness = roughness;
        Objects[index].albedo = albedo;
        Objects[index].emissive = 0;
        
        if (ior >= 0) {
            Objects[index].dielectric = 1;
            Objects[index].ior = ior;
        } else {
            Objects[index].dielectric = 0;
        }

        Objects[index].padding0[0] = 0.0f;
        Objects[index].padding0[1] = 0.0f;
        Objects[index].padding0[2] = 0.0f;
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
        
        Objects[index].dielectric = 0;

        Objects[index].padding0[0] = 0.0f;
        Objects[index].padding0[1] = 0.0f;
        Objects[index].padding0[2] = 0.0f;
        Objects[index].padding2[0] = 0.0f;
        Objects[index].padding2[1] = 0.0f;
    }
};