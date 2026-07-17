#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class VertexBuffer {
    public:
    GLuint VAO, VBO;
    GLsizeiptr size;

    VertexBuffer(const void *Data, GLsizeiptr Size, GLenum drawtype) : size(Size) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, Data, drawtype);
        glBindVertexArray(0);
    }

    ~VertexBuffer() {
        if (VBO != 0)
        glDeleteBuffers(1, &VBO);
        if (VAO != 0)
        glDeleteVertexArrays(1, &VAO);
    }

    VertexBuffer (VertexBuffer &&other) noexcept : VBO(other.VBO), VAO(other.VAO), size(other.size) {
        other.VAO = 0;
        other.VBO = 0;
        other.size = 0;
    }

    VertexBuffer(const VertexBuffer &other) = delete;
    VertexBuffer &operator=(const VertexBuffer &other) = delete;

    VertexBuffer &operator=(VertexBuffer &&other) noexcept {
        if (this != &other) {
            if (VBO != 0) glDeleteBuffers(1, &VBO);
            if (VAO != 0) glDeleteVertexArrays(1, &VAO);
            VBO = other.VBO;
            VAO = other.VAO;
            size = other.size;
            other.VAO = 0;
            other.VBO = 0;
            other.size = 0;
        }
        return *this;
    }

    void addAttribute(int index, int components, GLenum type, int floatsPerVertex, int floatOffset) { //index: Location number in layout. components: Total number of data before looping back. type: Data type. floatsPerVertex: How many 4 bytes until the next piece of data. floatOffset: How many 4 bytes is offset
        GLsizei stride = floatsPerVertex * sizeof(float); // Total bytes per data
        size_t offset = floatOffset * sizeof(float);

        glBindVertexArray(VAO);
        glVertexAttribPointer(index, components, type, GL_FALSE, stride, (void*)offset);
        glEnableVertexAttribArray(index);
        glBindVertexArray(0);
    }

    void bind() {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
    }
    
    void unbind() {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};