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
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include "objecthandler.h"
#include "computeshader.h"
#include "SSBO.h"
#include "bvh.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "model.h"
#include "texturebuffer.h"

using namespace std;

float quadVertices[] = {  
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

// Utility Functions

inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

double AspectRatio = 16.0/9.0;
unsigned int WIDTH = 1200;
unsigned int HEIGHT = float(WIDTH) / AspectRatio;
float FOV = 100.0;
float DeltaTime, LastFrame;
unsigned int FPSCounter, ShownFPS;
int FrameIndex = 0;

enum class Output {
    Raster,
    Raytrace,
    Combined,
    Comparison
};

Output OutputType = Output::Combined;

Camera CameraMain(glm::vec3(0.0, 0.0, 5.05));

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, Camera &camera) { //Spaghetti code GO
    const float cameraSpeed = 2.5f;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        FrameIndex = 0;
    }
    bool movements[6] = {false}; //W:0 S:1 A:2 D:3 SPACE:4 CONTROL:5
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        movements[0] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        movements[1] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        movements[2] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        movements[3] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        movements[4] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        movements[5] = true;
        FrameIndex = 0;
    }
    camera.keyboardprocess(movements, DeltaTime, cameraSpeed);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        OutputType = Output::Raster;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        OutputType = Output::Raytrace;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        OutputType = Output::Combined;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        OutputType = Output::Comparison;
    }
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
    FrameIndex = 0;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    FOV -= (float)yoffset;
    if (FOV < 1.0f)
        FOV = 1.0f;
    if (FOV > 170.0f)
        FOV = 170.0f;
    FrameIndex = 0;
}

int main () {
    srand(time(0));

    if (!glfwInit()) {
        cerr << "GLFW initialization failure\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

    GLuint RaytraceShaderAccumulationTexture;
    glGenTextures(1, &RaytraceShaderAccumulationTexture);

    glBindTexture(GL_TEXTURE_2D, RaytraceShaderAccumulationTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader ShaderSample("shader/sample.vert", "shader/sample.frag");

    VertexBuffer BufferQuad(&quadVertices, sizeof(quadVertices), GL_STATIC_DRAW);
    BufferQuad.addAttribute(0, 2, GL_FLOAT, 4, 0);
    BufferQuad.addAttribute(1, 2, GL_FLOAT, 4, 2);

    //  Framebuffer setup

    // PASS 1: G-BUFFER

    GLuint gBufferFBO;

    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

    GLuint gPosition, gNormal, gAlbedo, gRoughness, gDepth;

    //  gPosition
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    //  gNormal
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    //  gAlbedo
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

    //  gRoughness
    glGenTextures(1, &gRoughness);
    glBindTexture(GL_TEXTURE_2D, gRoughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, WIDTH, HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gRoughness, 0);

    //  gDepth
    glGenRenderbuffers(1, &gDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepth);

    GLuint attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cerr << "G-Buffer FBO is incomplete" << endl;
        return 1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader gBufferShader("shader/gBuffer.vert", "shader/gBuffer.frag");

    // PASS 2: RASTERIZATION & DIRECT LIGHT

    GLuint RasterFBO;

    glGenFramebuffers(1, &RasterFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, RasterFBO);

    GLuint RasterOutput;

    glGenTextures(1, &RasterOutput);
    glBindTexture(GL_TEXTURE_2D, RasterOutput);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RasterOutput, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cerr << "Rasterizer FBO is incomplete" << endl;
        return 1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader RasterShader("shader/raster.vert", "shader/raster.frag");
    RasterLightArray RasterLightHandler(RasterShader, "pointLights");

    // temporary standalone compute pass

    ComputeShader RaytraceShader("shader/raytrace.comp");

    //CORNELL BOX

    /*std::vector<Object> Objects;

    ComputeObjectArray uObjects(Objects);

    Triangle tempTri;
    
    tempTri.v0.Position = glm::vec3(1,2.49,-1); tempTri.v1.Position = glm::vec3(-1,2.49,-1); tempTri.v2.Position = glm::vec3(1,2.49,1);
    tempTri.v0.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, -1.0, 0.0);
    uObjects.addTriangleEmission(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(4.0));
    tempTri.v0.Position = glm::vec3(-1,2.49,-1); tempTri.v1.Position = glm::vec3(1,2.49,1); tempTri.v2.Position = glm::vec3(-1,2.49,1);
    tempTri.v0.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, -1.0, 0.0);
    uObjects.addTriangleEmission(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(4.0));

    tempTri.v0.Position = glm::vec3(2.5,2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,2.5,2.5); tempTri.v2.Position = glm::vec3(-2.5,2.5,-2.5);
    tempTri.v0.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, -1.0, 0.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);
    tempTri.v0.Position = glm::vec3(-2.5,2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,2.5,2.5); tempTri.v2.Position = glm::vec3(-2.5,2.5,2.5);
    tempTri.v0.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, -1.0, 0.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);

    tempTri.v0.Position = glm::vec3(2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(-2.5,-2.5,-2.5); tempTri.v2.Position = glm::vec3(2.5,-2.5,2.5);
    tempTri.v0.Normal = glm::vec3(0.0, 1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, 1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, 1.0, 0.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);
    tempTri.v0.Position = glm::vec3(-2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(-2.5,-2.5,2.5); tempTri.v2.Position = glm::vec3(2.5,-2.5,2.5);
    tempTri.v0.Normal = glm::vec3(0.0, 1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, 1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, 1.0, 0.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);

    tempTri.v0.Position = glm::vec3(2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,2.5,-2.5); tempTri.v2.Position = glm::vec3(-2.5,-2.5,-2.5);
    tempTri.v0.Normal = glm::vec3(0.0, 0.0, 1.0); tempTri.v1.Normal = glm::vec3(0.0, 0.0, 1.0); tempTri.v2.Normal = glm::vec3(0.0, 0.0, 1.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);
    tempTri.v0.Position = glm::vec3(-2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,2.5,-2.5); tempTri.v2.Position = glm::vec3(-2.5,2.5,-2.5);
    tempTri.v0.Normal = glm::vec3(0.0, 0.0, 1.0); tempTri.v1.Normal = glm::vec3(0.0, 0.0, 1.0); tempTri.v2.Normal = glm::vec3(0.0, 0.0, 1.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);

    tempTri.v0.Position = glm::vec3(2.5,2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,-2.5,-2.5); tempTri.v2.Position = glm::vec3(2.5,2.5,2.5);
    tempTri.v0.Normal = glm::vec3(-1.0, 0.0, 0.0); tempTri.v1.Normal = glm::vec3(-1.0, 0.0, 0.0); tempTri.v2.Normal = glm::vec3(-1.0, 0.0, 0.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(0.0, 1.0, 0.0), 1.0);
    tempTri.v0.Position = glm::vec3(2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,-2.5,2.5); tempTri.v2.Position = glm::vec3(2.5,2.5,2.5);
    tempTri.v0.Normal = glm::vec3(-1.0, 0.0, 0.0); tempTri.v1.Normal = glm::vec3(-1.0, 0.0, 0.0); tempTri.v2.Normal = glm::vec3(-1.0, 0.0, 0.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(0.0, 1.0, 0.0), 1.0);

    tempTri.v0.Position = glm::vec3(-2.5,2.5,-2.5); tempTri.v1.Position = glm::vec3(-2.5,2.5,2.5); tempTri.v2.Position = glm::vec3(-2.5,-2.5,-2.5);
    tempTri.v0.Normal = glm::vec3(1.0, 0.0, 0.0); tempTri.v1.Normal = glm::vec3(1.0, 0.0, 0.0); tempTri.v2.Normal = glm::vec3(1.0, 0.0, 0.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0, 0.0, 0.0), 1.0);
    tempTri.v0.Position = glm::vec3(-2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(-2.5,2.5,2.5); tempTri.v2.Position = glm::vec3(-2.5,-2.5,2.5);
    tempTri.v0.Normal = glm::vec3(1.0, 0.0, 0.0); tempTri.v1.Normal = glm::vec3(1.0, 0.0, 0.0); tempTri.v2.Normal = glm::vec3(1.0, 0.0, 0.0);
    uObjects.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0, 0.0, 0.0), 1.0);

    BVH mainBVH(Objects);*/

    Model model("assets/backpack2.obj");

    Triangle tempTri;
    
    tempTri.v0.Position = glm::vec3(1,2.49,-1); tempTri.v1.Position = glm::vec3(-1,2.49,-1); tempTri.v2.Position = glm::vec3(1,2.49,1);
    tempTri.v0.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, -1.0, 0.0);
    model.objectHandler.addTriangleEmission(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0));
    tempTri.v0.Position = glm::vec3(-1,2.49,-1); tempTri.v1.Position = glm::vec3(1,2.49,1); tempTri.v2.Position = glm::vec3(-1,2.49,1);
    tempTri.v0.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, -1.0, 0.0);
    model.objectHandler.addTriangleEmission(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0));

    tempTri.v0.Position = glm::vec3(2.5,2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,2.5,2.5); tempTri.v2.Position = glm::vec3(-2.5,2.5,-2.5);
    tempTri.v0.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, -1.0, 0.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);
    tempTri.v0.Position = glm::vec3(-2.5,2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,2.5,2.5); tempTri.v2.Position = glm::vec3(-2.5,2.5,2.5);
    tempTri.v0.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, -1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, -1.0, 0.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);

    tempTri.v0.Position = glm::vec3(2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(-2.5,-2.5,-2.5); tempTri.v2.Position = glm::vec3(2.5,-2.5,2.5);
    tempTri.v0.Normal = glm::vec3(0.0, 1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, 1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, 1.0, 0.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);
    tempTri.v0.Position = glm::vec3(-2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(-2.5,-2.5,2.5); tempTri.v2.Position = glm::vec3(2.5,-2.5,2.5);
    tempTri.v0.Normal = glm::vec3(0.0, 1.0, 0.0); tempTri.v1.Normal = glm::vec3(0.0, 1.0, 0.0); tempTri.v2.Normal = glm::vec3(0.0, 1.0, 0.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);

    tempTri.v0.Position = glm::vec3(2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,2.5,-2.5); tempTri.v2.Position = glm::vec3(-2.5,-2.5,-2.5);
    tempTri.v0.Normal = glm::vec3(0.0, 0.0, 1.0); tempTri.v1.Normal = glm::vec3(0.0, 0.0, 1.0); tempTri.v2.Normal = glm::vec3(0.0, 0.0, 1.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);
    tempTri.v0.Position = glm::vec3(-2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,2.5,-2.5); tempTri.v2.Position = glm::vec3(-2.5,2.5,-2.5);
    tempTri.v0.Normal = glm::vec3(0.0, 0.0, 1.0); tempTri.v1.Normal = glm::vec3(0.0, 0.0, 1.0); tempTri.v2.Normal = glm::vec3(0.0, 0.0, 1.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0), 1.0);

    tempTri.v0.Position = glm::vec3(2.5,2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,-2.5,-2.5); tempTri.v2.Position = glm::vec3(2.5,2.5,2.5);
    tempTri.v0.Normal = glm::vec3(-1.0, 0.0, 0.0); tempTri.v1.Normal = glm::vec3(-1.0, 0.0, 0.0); tempTri.v2.Normal = glm::vec3(-1.0, 0.0, 0.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(0.0, 1.0, 0.0), 1.0);
    tempTri.v0.Position = glm::vec3(2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(2.5,-2.5,2.5); tempTri.v2.Position = glm::vec3(2.5,2.5,2.5);
    tempTri.v0.Normal = glm::vec3(-1.0, 0.0, 0.0); tempTri.v1.Normal = glm::vec3(-1.0, 0.0, 0.0); tempTri.v2.Normal = glm::vec3(-1.0, 0.0, 0.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(0.0, 1.0, 0.0), 1.0);

    tempTri.v0.Position = glm::vec3(-2.5,2.5,-2.5); tempTri.v1.Position = glm::vec3(-2.5,2.5,2.5); tempTri.v2.Position = glm::vec3(-2.5,-2.5,-2.5);
    tempTri.v0.Normal = glm::vec3(1.0, 0.0, 0.0); tempTri.v1.Normal = glm::vec3(1.0, 0.0, 0.0); tempTri.v2.Normal = glm::vec3(1.0, 0.0, 0.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0, 0.0, 0.0), 1.0);
    tempTri.v0.Position = glm::vec3(-2.5,-2.5,-2.5); tempTri.v1.Position = glm::vec3(-2.5,2.5,2.5); tempTri.v2.Position = glm::vec3(-2.5,-2.5,2.5);
    tempTri.v0.Normal = glm::vec3(1.0, 0.0, 0.0); tempTri.v1.Normal = glm::vec3(1.0, 0.0, 0.0); tempTri.v2.Normal = glm::vec3(1.0, 0.0, 0.0);
    model.objectHandler.addTriangle(tempTri.v0, tempTri.v1, tempTri.v2, glm::vec3(1.0, 0.0, 0.0), 1.0);

    GBufferManager gBufferHandler(gBufferShader, model.objectHandler.Objects, false);

    BVH mainBVH(model.GetObjectVector());
    mainBVH.Build();
    mainBVH.Flatten();

    std::cout << "BVH Statistics" << std::endl;
    std::cout << "Nodes: " << mainBVH.GetBVH().size() << std::endl;
    std::cout << "Triangles: " << mainBVH.GetData().size() << std::endl;
    std::cout << "TriIndices: " << mainBVH.GetTriIndices().size() << std::endl;

    auto& flat = mainBVH.GetBVH();
    for (int i = 0; i < std::min(10, (int)flat.size()); i++) {
        std::cout << "Node " << i << ": "
                  << "Min=(" << flat[i].BBMin.x << ", " << flat[i].BBMin.y << ", " << flat[i].BBMin.z << "), "
                  << "Max=(" << flat[i].BBMax.x << ", " << flat[i].BBMax.y << ", " << flat[i].BBMax.z << "), "
                  << "Left=" << flat[i].LeftChild_or_Count << ", "
                  << "Right=" << flat[i].RightChild_or_Start << std::endl;
    }

    ShaderStorageBuffer BVHBuffer(mainBVH.GetBVH().data(), mainBVH.GetBVH().size() * sizeof(FlatNode), GL_STATIC_DRAW);
    ShaderStorageBuffer TriangleIndices(mainBVH.GetTriIndices().data(), mainBVH.GetTriIndices().size() * sizeof(int), GL_STATIC_DRAW);
    ShaderStorageBuffer ObjectData(mainBVH.GetData().data(), mainBVH.GetData().size() * sizeof(Object), GL_STATIC_DRAW);
    //ShaderStorageBuffer EmissionData(uObjects.LightIndices.data(), uObjects.LightIndices.size() * sizeof(int), GL_STATIC_DRAW);
    RaytraceShader.bind();

    RaytraceShader.setInt("gPosition", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);

    RaytraceShader.setInt("gNormal", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);

    RaytraceShader.setInt("gAlbedo", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);

    RaytraceShader.setInt("gRoughness", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gRoughness);

    BVHBuffer.bindToShader(0);
    TriangleIndices.bindToShader(1);
    ObjectData.bindToShader(2);
    
    RaytraceShader.setInt("EmissorSize", model.objectHandler.LightIndices.size());
    //ShaderCompute.setInt("EmissorSize", uObjects.LightIndices.size());
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window, CameraMain);
        FrameIndex++;

        if (FrameIndex == 1) {
            const float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            glBindTexture(GL_TEXTURE_2D, RaytraceShaderAccumulationTexture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, black);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        float CurrentFrame = glfwGetTime();
        DeltaTime = CurrentFrame - LastFrame;
        if (floor(CurrentFrame) != floor(LastFrame)) {
            ShownFPS = FPSCounter;
            FPSCounter = 0;
        }

        stringstream titlestring;
        titlestring << "Cornbread Program (FPS: " << ShownFPS << ", Frame index: " << FrameIndex << ", Rendering type: ";
        switch (OutputType) {
            case Output::Raster:
                titlestring << "Raster"; 
                break;
            case Output::Raytrace:
                titlestring << "Raytracing"; 
                break;
            case Output::Combined:
                titlestring << "Combined"; 
                break;
            case Output::Comparison:
                titlestring << "Comparison"; 
                break;
            default:
                titlestring << "Unknown"; 
                break;
        }
        titlestring << ")";

        glfwSetWindowTitle(window, titlestring.str().c_str()); 
        LastFrame = CurrentFrame;

        //cout << "Delta: " << DeltaTime * 1000 << " ms" << endl;

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective((float)glm::radians(FOV), (float)AspectRatio, 0.1f, 100.0f);

        CameraMain.updateCamera();
        
        glm::mat4 view = CameraMain.calculateView();

        //Pass 1

        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gBufferShader.use();

        glm::mat4 matmodel = glm::mat4(1.0f);
        gBufferShader.setMat4("model", matmodel);
        gBufferShader.setMat4("view", view);
        gBufferShader.setMat4("projection", projection);

        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(matmodel)));
        gBufferShader.setMat3("normalMatrix", normalMatrix);

        gBufferHandler.bindTextures(model.GetTextureArrayID());
        
        gBufferHandler.draw();

        glDisable(GL_DEPTH_TEST);

        //Pass 2

        glBindFramebuffer(GL_FRAMEBUFFER, RasterFBO);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        RasterShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        RasterShader.setInt("gPosition", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        RasterShader.setInt("gNormal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        RasterShader.setInt("gAlbedo", 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gRoughness);
        RasterShader.setInt("gRoughness", 3);

        RasterShader.setVec3("viewPos", CameraMain.position);

        RasterLightHandler.addLightPoint(0, glm::vec3(0.0f, 2.49, 0.0f), glm::vec3(1.0), 1.0f, 0.09f, 0.032f);
        //RasterLightHandler.addLightSpotlight(1, CameraMain.position, CameraMain.front, glm::vec3(1.0), 1.0, 1.10, 1.0f, 0.09f, 0.032f);

        BufferQuad.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Pass 3

        RaytraceShader.bind();

        glBindImageTexture(0, RaytraceShaderAccumulationTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        RaytraceShader.setInt("Width", WIDTH);
        RaytraceShader.setInt("Height", HEIGHT);
        RaytraceShader.setFloat("glfwTime", CurrentFrame);
        RaytraceShader.setInt("MSAAsamples", 1);
        RaytraceShader.setInt("MaximumDepth", 1);
        RaytraceShader.setInt("FrameIndex", FrameIndex);
        RaytraceShader.setVec3("CameraPos", CameraMain.position);

        //cout << "(" << CameraMain.position.x << ", " << CameraMain.position.y << ", " << CameraMain.position.z << ")\n";

        RaytraceShader.use((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

        //drawing to screen

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);  

        ShaderSample.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, RasterOutput);
        ShaderSample.setInt("Raster", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, RaytraceShaderAccumulationTexture);
        ShaderSample.setInt("Raytrace", 1);

        ShaderSample.setInt("OutputType", static_cast<int>(OutputType));

        BufferQuad.bind();

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        
        FPSCounter++;
    }
    glDeleteTextures(1, &RaytraceShaderAccumulationTexture);
    glfwTerminate();
    return 0;
}