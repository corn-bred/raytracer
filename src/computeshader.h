#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ComputeShader {
    public:
    GLuint ID;
    
    ComputeShader(const char* computeFilePath) {
        std::string computeCode;
        std::ifstream computeFile;

        computeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            computeFile.open(computeFilePath);

            std::stringstream ssCompute;

            ssCompute << computeFile.rdbuf();

            computeFile.close();

            computeCode = ssCompute.str();
        }
        catch(std::ifstream::failure e) {
            std::cout << "ERROR::SHADER::FILE_UNSUCCESSFULLY_READ\n";
        }

        const char *cShaderCode = computeCode.c_str();

        GLuint compute;
        int success;
        char infolog[512];

        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, NULL);
        glCompileShader(compute);
        glGetShaderiv(compute, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(compute, 512, NULL, infolog);
            std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infolog << std::endl;
        }

        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);

        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(ID,512, NULL,infolog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << std::endl;
        }

        glDeleteShader(compute);
    }

    void bind() {
        glUseProgram(ID);
    }

    void use(unsigned int dispatchX, unsigned int dispatchY, unsigned int dispatchZ, GLuint memoryBarrier = GL_ALL_BARRIER_BITS) {
        glUseProgram(ID);
        glDispatchCompute(dispatchX, dispatchY, dispatchZ);
        glMemoryBarrier(memoryBarrier);
    }


    void setBool(const std::string &unifName, bool val) {
        glUniform1i(glGetUniformLocation(ID, unifName.c_str()), (int)(val) );
    }
    void setInt(const std::string &unifName, int val) {
        glUniform1i(glGetUniformLocation(ID, unifName.c_str()), val);
    }
    void setFloat(const std::string &unifName, float val) {
        glUniform1f(glGetUniformLocation(ID, unifName.c_str()), val);
    }

    void setVec2(const std::string &name, const glm::vec2 &value) { 
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
    }
    
    void setVec3(const std::string &name, const glm::vec3 &value) { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }
    
    void setVec4(const std::string &name, const glm::vec4 &value) { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
    }
    
    void setMat2(const std::string &name, const glm::mat2 &mat) {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    void setMat3(const std::string &name, const glm::mat3 &mat) {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    void setMat4(const std::string &name, const glm::mat4 &mat) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
};