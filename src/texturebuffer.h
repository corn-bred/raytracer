#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

class TextureBuffer {
    GLuint texture;
    public:
    TextureBuffer(const char *texturepath, GLenum wrappingS = GL_MIRRORED_REPEAT, GLenum wrappingT = GL_MIRRORED_REPEAT, GLenum min = GL_NEAREST_MIPMAP_LINEAR, GLenum mag = GL_LINEAR, bool isFlippedVert = true) {
        glGenTextures(1, &texture);
        {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
            int width, height, nrchannels;
            stbi_set_flip_vertically_on_load(isFlippedVert);
            unsigned char *data = stbi_load(texturepath, &width, &height, &nrchannels, 0);
            if (data) {
                std::cerr << "SUCCESS: " << width << "x" << height << ", channels=" << nrchannels << std::endl;
                GLenum format;
                switch (nrchannels) {
                    case 1:
                    format = GL_RED; break;
                    case 2:
                    format = GL_RG; break;
                    case 3:
                    format = GL_RGB; break;
                    case 4:
                    format = GL_RGBA; break;
                    default:
                    stbi_image_free(data); return; //This will never happen unless I'm just that bad at coding
                }
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); 
                glGenerateMipmap(GL_TEXTURE_2D);
            } else {
                std::cerr << "Texture failed to load" << std::endl; 
                glDeleteTextures(1, &texture); 
                texture = 0; 
                return;
            }
            stbi_image_free(data);
        }
    }
    
    TextureBuffer() = default;

    ~TextureBuffer() {
        glDeleteTextures(1, &texture);
    }

    TextureBuffer (TextureBuffer &&other) noexcept : texture(other.texture) {
        other.texture = 0;
    }

    TextureBuffer &operator= (TextureBuffer &&other) noexcept {
        if(this != &other) {
            if (texture != 0) glDeleteTextures(1, &texture);
            texture = other.texture;
            other.texture = 0;
        }
        return *this;
    }

    TextureBuffer (TextureBuffer &other) = delete;
    TextureBuffer &operator= (TextureBuffer &other) = delete;

    void bindTexture(unsigned int ID) {
        glActiveTexture(GL_TEXTURE0 + ID);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
};