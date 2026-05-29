#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class VertexBuffer {
    public:
    GLuint VAO, VBO;
    const void *data;
    GLsizeiptr size;

    VertexBuffer(const void *Data, GLsizeiptr Size, GLenum drawtype) : data(Data), size(Size) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, data, drawtype);
        glBindVertexArray(0);
    }

    ~VertexBuffer() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void addAttribute(int index, int chunksize, int components, GLenum GLtype, GLsizei stride, size_t offset) {//arraysize: size of one full data block, index: index number for VAO, arraystride: the size that this index will take, GLtype: the enum that is for the variable type, stridelength: the size in bits on how much to jump, startingpoint: the starting position of variable intake
        glBindVertexArray(VAO);
        glVertexAttribPointer(index, components, GLtype, GL_FALSE, stride * chunksize, (void*)(offset * stride));
        glEnableVertexAttribArray(index);
        glBindVertexArray(0);
    }

    void bind() {
        glBindVertexArray(VAO);
    }
    
    void unbind() {
        glBindVertexArray(0);
    }
};