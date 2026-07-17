#pragma once

#include <iostream>
#include <glad/glad.h>

class ShaderStorageBuffer {
    public:
    GLuint SSBO = 0;
    GLsizeiptr Size = 0;
    GLenum DrawType = GL_DYNAMIC_DRAW;

    ShaderStorageBuffer(const void *data, GLsizeiptr size, GLenum drawtype) : Size(size), DrawType(drawtype) {
        glGenBuffers(1, &SSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, Size, data, DrawType);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    ShaderStorageBuffer(const ShaderStorageBuffer &other) = delete;
    ShaderStorageBuffer &operator=(const ShaderStorageBuffer &other) = delete;

    ~ShaderStorageBuffer() {
        if (SSBO != 0)
            glDeleteBuffers(1, &SSBO);
    }

    ShaderStorageBuffer (ShaderStorageBuffer &&other) noexcept {
        SSBO = other.SSBO;
        Size = other.Size;
        DrawType = other.DrawType;
        other.SSBO = 0;
        other.Size = 0;
    }

    ShaderStorageBuffer &operator=(ShaderStorageBuffer &&other) noexcept {
        if (this != &other) {
            if (SSBO != 0) glDeleteBuffers(1, &SSBO);
            SSBO = other.SSBO;
            Size = other.Size;
            DrawType = other.DrawType;
            other.SSBO = 0;
            other.Size = 0;
        }
        return *this;
    }

    void bindToShader(int index) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, SSBO);
    }

    void bind() {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    }

    void unbind() {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void updateData(const void *data, GLsizeiptr size) {
        if (size > Size) {
            bind();
            glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, DrawType);
            unbind();
            Size = size;
        } else {
            bind();
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
            unbind();
        }
    }
};