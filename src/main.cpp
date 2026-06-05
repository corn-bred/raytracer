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

double AspectRatio = 16.0/9.0;
unsigned int WIDTH = 1200;
unsigned int HEIGHT = WIDTH / AspectRatio;
float FOV = 100.0;
float DeltaTime, LastFrame;
unsigned int FPSCounter;

Camera CameraMain(glm::vec3(0.0, 0.0, 0.0));

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, Camera &camera) {
    const float cameraSpeed = 2.5f;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    bool movements[6] = {false}; //W:0 S:1 A:2 D:3 SPACE:4 CONTROL:5
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        movements[0] = true;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        movements[1] = true;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        movements[2] = true;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        movements[3] = true;
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        movements[4] = true;
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        movements[5] = true;
    camera.keyboardprocess(movements, DeltaTime, cameraSpeed);
}

float LastX = -1.0, LastY = -1.0;

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    float xoffset = 0;
    float yoffset = 0;
    if (LastX != -1.0 && LastY != -1.0) {
        xoffset = xpos - LastX;
        yoffset = LastY - ypos;
    }
    
    LastX = xpos;
    LastY = ypos;
    
    CameraMain.mouseprocess(xoffset, yoffset, GL_TRUE);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    FOV -= (float)yoffset;
    if (FOV < 1.0f)
        FOV = 1.0f;
    if (FOV > 170.0f)
        FOV = 170.0f;
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

    glfwSwapInterval(0);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    GLFWvidmode *mode = const_cast<GLFWvidmode*>(glfwGetVideoMode(monitor));
    glfwSetWindowPos(window, (mode->width - WIDTH)/2, (mode->height - HEIGHT)/2);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    glfwSetCursorPosCallback(window, mouseCallback);
    CameraMain.mouseprocess(0, 0, GL_TRUE);

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

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective((float)glm::radians(FOV), (float)AspectRatio, 0.1f, 100.0f);

        CameraMain.updateCamera();
        
        glm::mat4 view = CameraMain.calculateView();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ShaderQuad.use();

        ShaderQuad.setMat4("invView", glm::inverse(view));
        ShaderQuad.setMat4("invProjection", glm::inverse(projection));
        ShaderQuad.setVec3("CameraPos", CameraMain.position);
        ShaderQuad.setVec3("uObjects[0].center", glm::vec3(0,0,-1));
        ShaderQuad.setFloat("uObjects[0].radius", 0.5);
        ShaderQuad.setVec3("uObjects[1].center", glm::vec3(0,-100.5,-1));
        ShaderQuad.setFloat("uObjects[1].radius", 100);
        //cout << "(" << CameraMain.position.x << ", " << CameraMain.position.y << ", " << CameraMain.position.z << ")\n";

        BufferQuad.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);

        FPSCounter++;
    }
    glDeleteTextures(1, &FramebufferColourBuffer);
    glDeleteFramebuffers(1, &FramebufferMain);
    glfwTerminate();
    return 0;
}