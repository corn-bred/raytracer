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

    GLuint ComputeShaderAccumulationTexture;
    glGenTextures(1, &ComputeShaderAccumulationTexture);

    glBindTexture(GL_TEXTURE_2D, ComputeShaderAccumulationTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader ShaderSample("shader/vertSample.glsl", "shader/fragSample.glsl");

    VertexBuffer BufferQuad(&quadVertices, sizeof(quadVertices), GL_STATIC_DRAW);
    BufferQuad.addAttribute(0, 2, GL_FLOAT, 4, 0);
    BufferQuad.addAttribute(1, 2, GL_FLOAT, 4, 2);

    ComputeShader ShaderCompute("shader/main.comp");

    std::vector<Object> Objects;

    ObjectArray uObjects(Objects);

    ShaderCompute.bind();

    uObjects.addTriangleEmission(glm::vec3(1,2.49,-1), glm::vec3(-1,2.49,-1), glm::vec3(1,2.49,1), glm::vec3(4.0));
    uObjects.addTriangleEmission(glm::vec3(-1,2.49,-1), glm::vec3(1,2.49,1), glm::vec3(-1,2.49,1), glm::vec3(4.0));

    uObjects.addTriangle(glm::vec3(2.5,2.5,-2.5), glm::vec3(-2.5,2.5,-2.5), glm::vec3(2.5,2.5,2.5), glm::vec3(1.0), 1.0);
    uObjects.addTriangle(glm::vec3(-2.5,2.5,-2.5), glm::vec3(2.5,2.5,2.5), glm::vec3(-2.5,2.5,2.5), glm::vec3(1.0), 1.0);

    uObjects.addTriangle(glm::vec3(2.5,-2.5,-2.5), glm::vec3(-2.5,-2.5,-2.5), glm::vec3(2.5,-2.5,2.5), glm::vec3(1.0), 1.0);
    uObjects.addTriangle(glm::vec3(-2.5,-2.5,-2.5), glm::vec3(2.5,-2.5,2.5), glm::vec3(-2.5,-2.5,2.5), glm::vec3(1.0), 1.0);

    uObjects.addTriangle(glm::vec3(2.5,-2.5,-2.5), glm::vec3(-2.5,-2.5,-2.5), glm::vec3(2.5,2.5,-2.5), glm::vec3(1.0), 1.0);
    uObjects.addTriangle(glm::vec3(-2.5,-2.5,-2.5), glm::vec3(2.5,2.5,-2.5), glm::vec3(-2.5,2.5,-2.5), glm::vec3(1.0), 1.0);

    uObjects.addTriangle(glm::vec3(2.5,2.5,-2.5), glm::vec3(2.5,-2.5,-2.5), glm::vec3(2.5,2.5,2.5), glm::vec3(0.0, 1.0, 0.0), 1.0);
    uObjects.addTriangle(glm::vec3(2.5,-2.5,-2.5), glm::vec3(2.5,2.5,2.5), glm::vec3(2.5,-2.5,2.5), glm::vec3(0.0, 1.0, 0.0), 1.0);

    uObjects.addTriangle(glm::vec3(-2.5,2.5,-2.5), glm::vec3(-2.5,-2.5,-2.5), glm::vec3(-2.5,2.5,2.5), glm::vec3(1.0, 0.0, 0.0), 1.0);
    uObjects.addTriangle(glm::vec3(-2.5,-2.5,-2.5), glm::vec3(-2.5,2.5,2.5), glm::vec3(-2.5,-2.5,2.5), glm::vec3(1.0, 0.0, 0.0), 1.0);

    BVH mainBVH(Objects);
    mainBVH.Build();
    mainBVH.Flatten();

    ShaderStorageBuffer BVHBuffer(mainBVH.GetBVH().data(), mainBVH.GetBVH().size() * sizeof(FlatNode), GL_STATIC_DRAW);
    ShaderStorageBuffer TriangleIndices(mainBVH.GetTriIndices().data(), mainBVH.GetTriIndices().size() * sizeof(int), GL_STATIC_DRAW);
    ShaderStorageBuffer ObjectData(mainBVH.GetData().data(), mainBVH.GetData().size() * sizeof(Object), GL_STATIC_DRAW);
    ShaderCompute.bind();
    BVHBuffer.bindToShader(0);
    TriangleIndices.bindToShader(1);
    ObjectData.bindToShader(2);
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window, CameraMain);
        FrameIndex++;

        if (FrameIndex == 1) {
            const float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            glBindTexture(GL_TEXTURE_2D, ComputeShaderAccumulationTexture);
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
        titlestring << "Cornbread Program (FPS: " << ShownFPS << ", Frame index: " << FrameIndex <<")";
        glfwSetWindowTitle(window, titlestring.str().c_str()); 
        LastFrame = CurrentFrame;

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective((float)glm::radians(FOV), (float)AspectRatio, 0.1f, 100.0f);

        CameraMain.updateCamera();
        
        glm::mat4 view = CameraMain.calculateView();

        ShaderCompute.bind();

        glBindImageTexture(0, ComputeShaderAccumulationTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        ShaderCompute.setMat4("invView", glm::inverse(view));
        ShaderCompute.setMat4("invProjection", glm::inverse(projection));
        ShaderCompute.setVec3("CameraPos", CameraMain.position);
        ShaderCompute.setInt("Width", WIDTH);
        ShaderCompute.setInt("Height", HEIGHT);
        ShaderCompute.setFloat("glfwTime", CurrentFrame);
        ShaderCompute.setInt("MSAAsamples", 1);
        ShaderCompute.setInt("MaximumDepth", 100);
        ShaderCompute.setInt("FrameIndex", FrameIndex);
        ShaderCompute.setFloat("Roughness", CurrentFrame/1.0);
        ShaderCompute.setFloat("DefocusAngle", 0.5); 
        ShaderCompute.setFloat("FocusDist", 2.5);

        //cout << "(" << CameraMain.position.x << ", " << CameraMain.position.y << ", " << CameraMain.position.z << ")\n";

        ShaderCompute.use((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        
        
        ShaderSample.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ComputeShaderAccumulationTexture);
        ShaderSample.setInt("TextureAccumulation", 0);

        BufferQuad.bind();

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        
        FPSCounter++;
    }
    glDeleteTextures(1, &ComputeShaderAccumulationTexture);
    glfwTerminate();
    return 0;
}