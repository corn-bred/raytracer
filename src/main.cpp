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
unsigned int HEIGHT = WIDTH / AspectRatio;
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

    GLuint FramebufferMain;
    glGenFramebuffers(1, &FramebufferMain);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferMain);

    int ReadTexture = 0, WriteTexture = 1;
    GLuint FramebufferAccumulationTexture[2];
    glGenTextures(2, FramebufferAccumulationTexture);

    for(int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, FramebufferAccumulationTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader ShaderQuad("shader/vert.glsl", "shader/frag.glsl");
    Shader ShaderSample("shader/vertSample.glsl", "shader/fragSample.glsl");

    VertexBuffer BufferQuad(&quadVertices, sizeof(quadVertices), GL_STATIC_DRAW);
    BufferQuad.addAttribute(0, 4, 2, GL_FLOAT, sizeof(float), 0);
    BufferQuad.addAttribute(1, 4, 2, GL_FLOAT, sizeof(float), 2);

    ObjectArray uObjects(ShaderQuad, "uObjects");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window, CameraMain);
        FrameIndex++;

        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferMain);

        if (FrameIndex == 1) {
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
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

        ShaderQuad.use();

        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferMain);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FramebufferAccumulationTexture[WriteTexture], 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FramebufferAccumulationTexture[ReadTexture]);
        ShaderQuad.setInt("TextureAccumulation", 0);

        ShaderQuad.setMat4("invView", glm::inverse(view));
        ShaderQuad.setMat4("invProjection", glm::inverse(projection));
        ShaderQuad.setVec3("CameraPos", CameraMain.position);
        ShaderQuad.setInt("Width", WIDTH);
        ShaderQuad.setInt("Height", HEIGHT);
        ShaderQuad.setFloat("glfwTime", CurrentFrame);
        ShaderQuad.setInt("MSAAsamples", 1);
        ShaderQuad.setInt("MaximumDepth", 10);
        ShaderQuad.setInt("FrameIndex", FrameIndex);
        ShaderQuad.setFloat("Roughness", CurrentFrame/1.0);
        ShaderQuad.setFloat("DefocusAngle", 1); //sigma toes
        ShaderQuad.setFloat("FocusDist", 1.0);

        uObjects.addTriangleEmission(0, glm::vec3(1,2.4,-1), glm::vec3(-1,2.4,-1), glm::vec3(1,2.4,1), glm::vec3(4.0));
        uObjects.addTriangleEmission(1, glm::vec3(-1,2.4,-1), glm::vec3(1,2.4,1), glm::vec3(-1,2.4,1), glm::vec3(4.0));

        uObjects.addTriangle(2, glm::vec3(2.5,2.5,-2.5), glm::vec3(-2.5,2.5,-2.5), glm::vec3(2.5,2.5,2.5), glm::vec3(1.0), 1.0);
        uObjects.addTriangle(3, glm::vec3(-2.5,2.5,-2.5), glm::vec3(2.5,2.5,2.5), glm::vec3(-2.5,2.5,2.5), glm::vec3(1.0), 1.0);

        uObjects.addTriangle(4, glm::vec3(2.5,-2.5,-2.5), glm::vec3(-2.5,-2.5,-2.5), glm::vec3(2.5,-2.5,2.5), glm::vec3(1.0), 1.0);
        uObjects.addTriangle(5, glm::vec3(-2.5,-2.5,-2.5), glm::vec3(2.5,-2.5,2.5), glm::vec3(-2.5,-2.5,2.5), glm::vec3(1.0), 1.0);

        uObjects.addTriangle(6, glm::vec3(2.5,-2.5,-2.5), glm::vec3(-2.5,-2.5,-2.5), glm::vec3(2.5,2.5,-2.5), glm::vec3(1.0), 1.0);
        uObjects.addTriangle(7, glm::vec3(-2.5,-2.5,-2.5), glm::vec3(2.5,2.5,-2.5), glm::vec3(-2.5,2.5,-2.5), glm::vec3(1.0), 1.0);

        uObjects.addTriangle(8, glm::vec3(2.5,2.5,-2.5), glm::vec3(2.5,-2.5,-2.5), glm::vec3(2.5,2.5,2.5), glm::vec3(0.0, 1.0, 0.0), 1.0);
        uObjects.addTriangle(9, glm::vec3(2.5,-2.5,-2.5), glm::vec3(2.5,2.5,2.5), glm::vec3(2.5,-2.5,2.5), glm::vec3(0.0, 1.0, 0.0), 1.0);

        uObjects.addTriangle(10, glm::vec3(-2.5,2.5,-2.5), glm::vec3(-2.5,-2.5,-2.5), glm::vec3(-2.5,2.5,2.5), glm::vec3(1.0, 0.0, 0.0), 1.0);
        uObjects.addTriangle(11, glm::vec3(-2.5,-2.5,-2.5), glm::vec3(-2.5,2.5,2.5), glm::vec3(-2.5,-2.5,2.5), glm::vec3(1.0, 0.0, 0.0), 1.0);

        uObjects.addSphere(12, glm::vec3(1, -1.75, 1), 0.75, glm::vec3(1.0), 0.0, 1.5);
        uObjects.addSphere(13, glm::vec3(-1, -1.75, -1), 0.75, glm::vec3(1.0), 0.2);
        //cout << "(" << CameraMain.position.x << ", " << CameraMain.position.y << ", " << CameraMain.position.z << ")\n";

        BufferQuad.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        swap(ReadTexture, WriteTexture);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ShaderSample.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FramebufferAccumulationTexture[ReadTexture]);
        ShaderSample.setInt("TextureAccumulation", 0);

        BufferQuad.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        
        FPSCounter++;
    }
    glDeleteTextures(2, FramebufferAccumulationTexture);
    glDeleteFramebuffers(1, &FramebufferMain);
    glfwTerminate();
    return 0;
}