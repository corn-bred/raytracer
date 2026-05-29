#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include "shaders.h"
#include "vertexbuffer.h"

using namespace std;

float quadVertices[] = {  
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};	

unsigned int WIDTH = 800, HEIGHT = 600;
float fov = 100;
float DeltaTime, LastFrame;
unsigned int FPSCounter;

Camera CameraMain(glm::vec3(0.0, 0.0, 0.0));

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, Camera camera) {
    const float cameraSpeed = 2.5f;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    bool movements[6] = {false}; //W:0 S:1 A:2 D:3 SPACE:4 CONTROL:5
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        movements[0] = GLFW_PRESS;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        movements[1] = GLFW_PRESS;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        movements[2] = GLFW_PRESS;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        movements[3] = GLFW_PRESS;
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        movements[4] = GLFW_PRESS;
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        movements[5] = GLFW_PRESS;
    
    camera.keyboardprocess(movements, DeltaTime, cameraSpeed);
}

float LastX, LastY;

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {

    float xoffset = xpos - LastX;
    float yoffset = LastY - ypos;
    LastX = xpos;
    LastY = ypos;

    CameraMain.mouseprocess(xoffset, yoffset, GL_TRUE);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}

int main () {
    if (!glfwInit()) {
        cerr << "GLFW initialization failure\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Raytracer", NULL, NULL);
    if (!window) {
        cerr << "GLFW window creation failure\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    glfwSetCursorPosCallback(window, mouseCallback);
    
    glfwSetScrollCallback(window, scrollCallback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        cerr << "GLAD initialization failure\n";
        glfwTerminate();
        return 1;
    }

    GLuint FramebufferMain;
    glGenFramebuffers(1, &FramebufferMain);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferMain);

    GLuint FramebufferColourBuffer;
    glGenTextures(1, &FramebufferColourBuffer);
    glBindTexture(GL_TEXTURE_2D, FramebufferColourBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FramebufferColourBuffer, 0);

    if (!glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        cerr << "Framebuffer is incomplete\n";
        glDeleteTextures(1, &FramebufferColourBuffer);
        glDeleteFramebuffers(1, &FramebufferMain);
        glfwTerminate();
        return 1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader ShaderQuad("shader/vert.glsl", "shader/frag.glsl");

    VertexBuffer BufferQuad(&quadVertices, sizeof(quadVertices), GL_STATIC_DRAW);
    BufferQuad.addAttribute(0, 4, 2, GL_FLOAT, sizeof(float), 0);
    BufferQuad.addAttribute(1, 4, 2, GL_FLOAT, sizeof(float), 2);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window, CameraMain);

        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferMain);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        float CurrentFrame = glfwGetTime();
        DeltaTime = CurrentFrame - LastFrame;
        if (floor(CurrentFrame) != floor(LastFrame)) {
            stringstream titlestring;
            titlestring << "Cornbread Program (FPS: " << FPSCounter << ")";
            glfwSetWindowTitle(window, titlestring.str().c_str()); 
            FPSCounter = 0;
        }
        LastFrame = CurrentFrame;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ShaderQuad.use();
        BufferQuad.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
    }
    glDeleteTextures(1, &FramebufferColourBuffer);
    glDeleteFramebuffers(1, &FramebufferMain);
    glfwTerminate();
    return 0;
}