#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Camera.h"
#include "Shader.h"
#include "stb_image.h"

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f),
              glm::vec3(0.0f, 0.0f, 1.0f),
              glm::vec3(0.0f, 1.0f, 0.0f),
              45.0f, 0.0f, 90.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cameraMouseCaptured = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float exposure = 0.2f;

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
unsigned int quadVAO = 0;
unsigned int quadVBO = 0;
unsigned int tunnelVAO = 0;
unsigned int tunnelVBO = 0;

static std::string resourcePath(const std::string& relativePath)
{
    return std::string(PROJECT_SOURCE_DIR) + "/" + relativePath;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void renderCube();
void renderQuad();
void renderTunnel();

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL HDR Clamp Demo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Shader shader(resourcePath("src/vertexShader/lighting.vs").c_str(),
                  resourcePath("src/fragmentShader/lighting.fs").c_str());
    Shader frameShader(resourcePath("src/vertexShader/frameBufferShader.vs").c_str(),
                    resourcePath("src/fragmentShader/frameBufferShader.fs").c_str());
    unsigned int brickTexture = loadTexture(resourcePath("src/texture/texture_brick.jpg").c_str());

    unsigned int hdrFBO;
    glGenFramebuffers(1,&hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,hdrFBO);

    unsigned int hdrTexture;
    glGenTextures(1,&hdrTexture);
    glBindTexture(GL_TEXTURE_2D,hdrTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,SCR_WIDTH,SCR_HEIGHT,0,GL_RGB,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,hdrTexture,0);

    std::vector<glm::vec3> lightPositions;
    lightPositions.push_back(glm::vec3(0.0f, 0.0f, 49.5f));
    lightPositions.push_back(glm::vec3(-1.4f, -1.9f, 9.0f));
    lightPositions.push_back(glm::vec3(0.0f, -1.8f, 4.0f));
    lightPositions.push_back(glm::vec3(0.8f, -1.7f, 6.0f));

    std::vector<glm::vec3> lightColors;
    lightColors.push_back(glm::vec3(200.0f, 200.0f, 200.0f));
    lightColors.push_back(glm::vec3(0.1f, 0.0f, 0.0f));
    lightColors.push_back(glm::vec3(0.0f, 0.0f, 0.2f));
    lightColors.push_back(glm::vec3(0.0f, 0.1f, 0.0f));

    shader.use();
    shader.setInt("diffuseTexture", 0);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Debug");
        ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::SliderFloat("Exposure", &exposure, 0.0f, 5.0f);
        ImGui::End();

        glBindFramebuffer(GL_FRAMEBUFFER,hdrFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov),
                                                static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
                                                0.1f,
                                                100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);
        for (unsigned int i = 0; i < lightPositions.size(); ++i)
        {
            char uniformName[32];
            std::snprintf(uniformName, sizeof(uniformName), "lights[%u].Position", i);
            shader.setVec3(uniformName, lightPositions[i]);
            std::snprintf(uniformName, sizeof(uniformName), "lights[%u].Color", i);
            shader.setVec3(uniformName, lightColors[i]);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickTexture);

        shader.setMat4("model", glm::mat4(1.0f));
        shader.setBool("inverseNormals", false);
        shader.setBool("useTexture", true);
        shader.setFloat("textureScale", 1.0f);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        shader.setVec3("emissiveColor", glm::vec3(0.0f));
        renderTunnel();

        glm::mat4 lamp = glm::mat4(1.0f);
        lamp = glm::translate(lamp, glm::vec3(0.0f, 0.0f, 49.2f));
        lamp = glm::scale(lamp, glm::vec3(1.4f, 1.4f, 0.08f));
        shader.setMat4("model", lamp);
        shader.setBool("inverseNormals", false);
        shader.setBool("useTexture", false);
        shader.setFloat("textureScale", 1.0f);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        shader.setVec3("emissiveColor", glm::vec3(25.0f, 25.0f, 25.0f));
        renderCube();

        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        frameShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,hdrTexture);
        frameShader.setInt("hdrTexture",0);
        frameShader.setFloat("exposure", exposure);
        renderQuad();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &tunnelVAO);
    glDeleteBuffers(1, &tunnelVBO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(deltaTime, Camera_Movement::FORWARD);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(deltaTime, Camera_Movement::BACKWARD);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(deltaTime, Camera_Movement::LEFT);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(deltaTime, Camera_Movement::RIGHT);
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true;
        cameraMouseCaptured = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
        cameraMouseCaptured = false;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!cameraMouseCaptured)
    {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
        {
            format = GL_RED;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }

    stbi_image_free(data);
    return textureID;
}

void renderCube()
{
    if (cubeVAO == 0)
    {
        float vertices[] = {
            -1.0f,-1.0f,-1.0f, 0.0f, 0.0f,-1.0f, 0.0f, 0.0f,
             1.0f, 1.0f,-1.0f, 0.0f, 0.0f,-1.0f, 1.0f, 1.0f,
             1.0f,-1.0f,-1.0f, 0.0f, 0.0f,-1.0f, 1.0f, 0.0f,
             1.0f, 1.0f,-1.0f, 0.0f, 0.0f,-1.0f, 1.0f, 1.0f,
            -1.0f,-1.0f,-1.0f, 0.0f, 0.0f,-1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f,-1.0f, 0.0f, 0.0f,-1.0f, 0.0f, 1.0f,

            -1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
             1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
             1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            -1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

            -1.0f, 1.0f, 1.0f,-1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f,-1.0f,-1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -1.0f,-1.0f,-1.0f,-1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -1.0f,-1.0f,-1.0f,-1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -1.0f,-1.0f, 1.0f,-1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 1.0f,-1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

             1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
             1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
             1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
             1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
             1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
             1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

            -1.0f,-1.0f,-1.0f, 0.0f,-1.0f, 0.0f, 0.0f, 1.0f,
             1.0f,-1.0f,-1.0f, 0.0f,-1.0f, 0.0f, 1.0f, 1.0f,
             1.0f,-1.0f, 1.0f, 0.0f,-1.0f, 0.0f, 1.0f, 0.0f,
             1.0f,-1.0f, 1.0f, 0.0f,-1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f,-1.0f, 1.0f, 0.0f,-1.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,-1.0f,-1.0f, 0.0f,-1.0f, 0.0f, 0.0f, 1.0f,

            -1.0f, 1.0f,-1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
             1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
             1.0f, 1.0f,-1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f,-1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderTunnel()
{
    if (tunnelVAO == 0)
    {
        const float x = 2.5f;
        const float y = 2.5f;
        const float nearZ = -2.5f;
        const float farZ = 52.5f;
        const float lengthRepeat = 22.0f;
        const float crossRepeat = 2.0f;

        float vertices[] = {
            // floor, normal points into the tunnel
            -x, -y, nearZ,  0.0f, 1.0f, 0.0f,  0.0f,         0.0f,
             x, -y, nearZ,  0.0f, 1.0f, 0.0f,  crossRepeat,  0.0f,
             x, -y, farZ,   0.0f, 1.0f, 0.0f,  crossRepeat,  lengthRepeat,
             x, -y, farZ,   0.0f, 1.0f, 0.0f,  crossRepeat,  lengthRepeat,
            -x, -y, farZ,   0.0f, 1.0f, 0.0f,  0.0f,         lengthRepeat,
            -x, -y, nearZ,  0.0f, 1.0f, 0.0f,  0.0f,         0.0f,

            // ceiling
            -x,  y, farZ,   0.0f,-1.0f, 0.0f,  0.0f,         lengthRepeat,
             x,  y, farZ,   0.0f,-1.0f, 0.0f,  crossRepeat,  lengthRepeat,
             x,  y, nearZ,  0.0f,-1.0f, 0.0f,  crossRepeat,  0.0f,
             x,  y, nearZ,  0.0f,-1.0f, 0.0f,  crossRepeat,  0.0f,
            -x,  y, nearZ,  0.0f,-1.0f, 0.0f,  0.0f,         0.0f,
            -x,  y, farZ,   0.0f,-1.0f, 0.0f,  0.0f,         lengthRepeat,

            // left wall
            -x, -y, farZ,   1.0f, 0.0f, 0.0f,  0.0f,         lengthRepeat,
            -x,  y, farZ,   1.0f, 0.0f, 0.0f,  crossRepeat,  lengthRepeat,
            -x,  y, nearZ,  1.0f, 0.0f, 0.0f,  crossRepeat,  0.0f,
            -x,  y, nearZ,  1.0f, 0.0f, 0.0f,  crossRepeat,  0.0f,
            -x, -y, nearZ,  1.0f, 0.0f, 0.0f,  0.0f,         0.0f,
            -x, -y, farZ,   1.0f, 0.0f, 0.0f,  0.0f,         lengthRepeat,

            // right wall
             x, -y, nearZ, -1.0f, 0.0f, 0.0f,  0.0f,         0.0f,
             x,  y, nearZ, -1.0f, 0.0f, 0.0f,  crossRepeat,  0.0f,
             x,  y, farZ,  -1.0f, 0.0f, 0.0f,  crossRepeat,  lengthRepeat,
             x,  y, farZ,  -1.0f, 0.0f, 0.0f,  crossRepeat,  lengthRepeat,
             x, -y, farZ,  -1.0f, 0.0f, 0.0f,  0.0f,         lengthRepeat,
             x, -y, nearZ, -1.0f, 0.0f, 0.0f,  0.0f,         0.0f,

            // bright end wall, still textured so overexposure can wipe out detail
            -x, -y, farZ,   0.0f, 0.0f,-1.0f,  0.0f,         0.0f,
             x, -y, farZ,   0.0f, 0.0f,-1.0f,  crossRepeat,  0.0f,
             x,  y, farZ,   0.0f, 0.0f,-1.0f,  crossRepeat,  crossRepeat,
             x,  y, farZ,   0.0f, 0.0f,-1.0f,  crossRepeat,  crossRepeat,
            -x,  y, farZ,   0.0f, 0.0f,-1.0f,  0.0f,         crossRepeat,
            -x, -y, farZ,   0.0f, 0.0f,-1.0f,  0.0f,         0.0f
        };

        glGenVertexArrays(1, &tunnelVAO);
        glGenBuffers(1, &tunnelVBO);
        glBindVertexArray(tunnelVAO);
        glBindBuffer(GL_ARRAY_BUFFER, tunnelVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(tunnelVAO);
    glDrawArrays(GL_TRIANGLES, 0, 30);
    glBindVertexArray(0);
}

void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
