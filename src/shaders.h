#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
    public:
    GLuint ID;
    
    Shader(const char* vertexFilePath, const char* fragmentFilePath);

    void use();

    void setBool(const std::string &unifName, bool val);
    void setInt(const std::string &unifName, int val);
    void setFloat(const std::string &unifName, float val);

    void setVec2(const std::string &name, const glm::vec2 &value);
    void setVec2(const std::string &name, float x, float y);
    
    void setVec3(const std::string &name, const glm::vec3 &value);
    void setVec3(const std::string &name, float x, float y, float z);
    
    void setVec4(const std::string &name, const glm::vec4 &value);
    void setVec4(const std::string &name, float x, float y, float z, float w);
    
    void setMat2(const std::string &name, const glm::mat2 &mat);
    
    void setMat3(const std::string &name, const glm::mat3 &mat);
    
    void setMat4(const std::string &name, const glm::mat4 &mat);
};