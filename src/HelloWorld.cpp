#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <random>
#include <vector>
#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//窗口大小
float width = 1920;
float height = 1080;

//相机设置全局变量
Camera camera(glm::vec3(0.0f,0.0f,2.5f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f),45.0f,0.0f,-90.0f);
float lastX = width/2.0f;
float lastY = height/2.0f;
bool firstMouse = true;
bool cameraMouseCaptured = false;
float deltaTime = 0.0f; //当前帧与上一帧的时间差
float lastFrameTime = 0.0f; //上一帧的时间


static std::string resourcePath(const std::string& relativePath)
{
    return std::string(PROJECT_SOURCE_DIR) + "/" + relativePath;
}

//resize回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0,0,width,height);
}

//处理输入，按esc退出
void processInput(GLFWwindow* window, float deltaTime){
    if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS){
        glfwSetWindowShouldClose(window,true);
    }

    if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS){
        camera.ProcessKeyboard(deltaTime,Camera_Movement::BACKWARD);
    }
    if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS){
        camera.ProcessKeyboard(deltaTime,Camera_Movement::FORWARD);
    }
    if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS){
        camera.ProcessKeyboard(deltaTime,Camera_Movement::LEFT);
    }
    if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS){
        camera.ProcessKeyboard(deltaTime,Camera_Movement::RIGHT);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
        cameraMouseCaptured = false;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        cameraMouseCaptured = true;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if(!cameraMouseCaptured){
        return;
    }

    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; //注意这里是相反的，因为y坐标是从底部往顶部依次减小的
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset,yoffset);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera.ProcessMouseScroll(yoffset);
}

static unsigned int TextureFromFile(const char *path, const std::string &directory){
    std::string filename = std::string(path);
    filename = directory + "/" + filename;

    unsigned int textureId;
    glGenTextures(1,&textureId);
    int width,height,nrChannels;
    unsigned char* data = stbi_load(filename.c_str(),&width,&height,&nrChannels,0);
    if(data==NULL){
        std::cout<<"Texture failed to load: "<<filename<<std::endl;
        return 0;
    }
    glBindTexture(GL_TEXTURE_2D,textureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    GLenum format = GL_RGB;
    if(nrChannels == 1) format = GL_RED;
    else if(nrChannels == 2) format = GL_RG;
    else if(nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return textureId;
}

unsigned int loadCubemap(std::vector<std::string> faces){
    unsigned int textureID;
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width,height,nrchannels;

    for(unsigned int i = 0; i < faces.size(); i++){
       unsigned char *data = stbi_load(faces[i].c_str(),&width,&height,&nrchannels,0);
       if(data){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
        stbi_image_free(data);
       }
       else if(!data){
        std::cout<<"Cube map loaded failed"<<std::endl;
        stbi_image_free(data);
       }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    return textureID;
}

float cubeVertices[] = {
    // positions          // normals           // texture coords
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
};

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

unsigned int sphereVAO = 0;
unsigned int sphereVBO = 0;
unsigned int sphereEBO = 0;
unsigned int sphereIndexCount = 0;
const unsigned int X_SEGMENTS = 32;
const unsigned int Y_SEGMENTS = 16;

const int NR_POINT_LIGHTS = 4;
glm::vec3 lightPositions[NR_POINT_LIGHTS];
glm::vec3 lightColors[NR_POINT_LIGHTS];

float constant = 1;
float linear = 0.09f;
float quadratic = 0.032f;

void initLights(){
    static bool initialized = false;
    if(initialized){
        return;
    }

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        int x = i % 8;
        int z = i / 8;
        float height = 0.35f + float((i * 7) % 5) * 0.38f;

        lightPositions[i] = glm::vec3(
            -4.2f + float(x) * 1.2f,
            height,
            -4.2f + float(z) * 1.2f
        );

        lightColors[i] = glm::vec3(
            1.4f + float((i * 13) % 9) * 0.35f,
            1.2f + float((i * 17) % 8) * 0.32f,
            1.6f + float((i * 19) % 7) * 0.38f
        );
    }

    lightPositions[0] = glm::vec3(0.0f, 1.5f, 1.5f);
    lightPositions[1] = glm::vec3(2.8f, 0.4f, -1.6f);
    lightColors[0] = glm::vec3(4.0f, 3.2f, 1.8f);
    lightColors[1] = glm::vec3(1.2f, 2.2f, 4.0f);

    initialized = true;
}

void initSphere(){
    if(sphereVAO != 0){
        return;
    }

    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    const float PI = 3.14159265359f;

    for(unsigned int y = 0; y <= Y_SEGMENTS; y++){
        for(unsigned int x = 0; x <= X_SEGMENTS; x++){
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;

            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
            sphereVertices.push_back(xSegment);
            sphereVertices.push_back(ySegment);
        }
    }

    for(unsigned int y = 0; y < Y_SEGMENTS; y++){
        for(unsigned int x = 0; x < X_SEGMENTS; x++){
            unsigned int i0 = y * (X_SEGMENTS + 1) + x;
            unsigned int i1 = (y + 1) * (X_SEGMENTS + 1) + x;
            unsigned int i2 = (y + 1) * (X_SEGMENTS + 1) + x + 1;
            unsigned int i3 = y * (X_SEGMENTS + 1) + x + 1;

            sphereIndices.push_back(i0);
            sphereIndices.push_back(i1);
            sphereIndices.push_back(i2);

            sphereIndices.push_back(i0);
            sphereIndices.push_back(i2);
            sphereIndices.push_back(i3);
        }
    }

    sphereIndexCount = sphereIndices.size();

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void initCube(){
    if(cubeVAO != 0){
        return;
    }

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void drawBoxes(Shader &shader){
    initCube();
    shader.setBool("isLight", false);

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(-1.8f, 0.0f, -1.0f),
        glm::vec3(1.6f, -0.2f, 1.1f),
        glm::vec3(0.0f, 1.0f, -2.2f)
    };
    glm::vec3 cubeScales[] = {
        glm::vec3(6.0f, 0.25f, 6.0f),
        glm::vec3(0.8f),
        glm::vec3(0.6f),
        glm::vec3(0.7f, 0.7f, 0.7f)
    };

    glBindVertexArray(cubeVAO);
    for(int i = 0; i < 4; i++){
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        model = glm::scale(model, cubeScales[i]);
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}

void drawLights(Shader &shader){
    initCube();
    shader.setBool("isLight", true);

    glBindVertexArray(cubeVAO);
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, lightPositions[i]);
        model = glm::scale(model, glm::vec3(0.16f));
        shader.setMat4("model", model);
        shader.setVec3("lightColor", lightColors[i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
    shader.setBool("isLight", false);
}

void drawLightsVolume(Shader &shader,float coefficient){
    initSphere();
    shader.setBool("isLight", true);

    glBindVertexArray(sphereVAO);
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        float maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].x, lightColors[i].y), lightColors[i].z);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, lightPositions[i]);
        float radius = (-linear + glm::sqrt(pow(linear, 2) - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2 * quadratic);
        radius = radius*coefficient;
        shader.setVec3("pointLight.Position", lightPositions[i]);
        shader.setVec3("pointLight.Ambient", lightColors[i] * 0.2f);
        shader.setVec3("pointLight.Diffuse", lightColors[i]);
        shader.setVec3("pointLight.Specular", lightColors[i]);
        shader.setFloat("pointLight.Constant", constant);
        shader.setFloat("pointLight.Linear", linear);
        shader.setFloat("pointLight.Quadratic", quadratic);
        model = glm::scale(model, glm::vec3(radius));
        shader.setMat4("model", model);
        shader.setFloat("pointLight.radius", radius);
        glDrawElements(GL_TRIANGLES,sphereIndexCount,GL_UNSIGNED_INT,0);
    }
    glBindVertexArray(0);
    shader.setBool("isLight", false);
}

float quadVertices[] = {
    // positions   // texture coords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f
};

unsigned int quadVAO = 0;
unsigned int quadVBO = 0;

void drawQuad(){
    if(quadVAO == 0){
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


int main(){
    //初始化Glfw，使用主版本号3，次版本3
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    //使用核心模式
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* window = glfwCreateWindow(width,height,"LearnOpenGL",NULL,NULL);
    if(window==NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    //初始化glad,通过glfw的getprocaddress去找到所有的opengl函数地址
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialized GLAD" << std::endl;
        return -1;
    }
    
    //注册窗口改变的回调函数
    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
    //设置窗口大小
    glViewport(0,0,width,height);

    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window,mouse_callback);
    glfwSetScrollCallback(window,scroll_callback);

    stbi_set_flip_vertically_on_load(true);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //--------G-BufferFBO-----------
    unsigned int gBuffer;
    glGenFramebuffers(1,&gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER,gBuffer);

    //--------gPosition------------
    unsigned int gPosition;
    glGenTextures(1,&gPosition);
    glBindTexture(GL_TEXTURE_2D,gPosition);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,width,height,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,gPosition,0);

    //--------gNormal------------
    unsigned int gNormal;
    glGenTextures(1,&gNormal);
    glBindTexture(GL_TEXTURE_2D,gNormal);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,width,height,0,GL_RGB,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,gNormal,0);

    //--------gAlbedoSpecular------------
    unsigned int gAlbedoSpecular;
    glGenTextures(1,&gAlbedoSpecular);
    glBindTexture(GL_TEXTURE_2D,gAlbedoSpecular);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,gAlbedoSpecular,0);

    //---------gDepth------------
    unsigned int gDepth;
    glGenRenderbuffers(1,&gDepth);
    glBindRenderbuffer(GL_RENDERBUFFER,gDepth);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,width,height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,gDepth);

    GLuint attachments[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3,attachments);

    //--------HDR lighting accumulation FBO------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int hdrColorBuffer;
    glGenTextures(1, &hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorBuffer, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "HDR Framebuffer not complete" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    //----------------sample kernel---------------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0,1.0);//random float from 0.0 to 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for(GLuint i = 0; i <64; i++){
        glm::vec3 sample(
            randomFloats(generator)*2.0 -1.0,
            randomFloats(generator)*2.0 -1.0,
            randomFloats(generator)
        );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) /64;
        //-----前面几个应为scale会把长度变得很小，从而接近原点
        scale = glm::mix(0.1f,1.0f,scale *scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    //-------------ssaonoise-----------------
    std::vector<glm::vec3> ssaoNoise;
    for (GLuint i = 0; i<16; i++){
        glm::vec3 noise(
            randomFloats(generator)*2.0 -1.0,
            randomFloats(generator)*2.0 -1.0,
            0.0f
        );
        ssaoNoise.push_back(noise);;
    }

    unsigned int noiseTexture;
    glGenTextures(1,&noiseTexture);
    glBindTexture(GL_TEXTURE_2D,noiseTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,4,4,0,GL_RGB,GL_FLOAT,ssaoNoise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //----------需要存储ssao计算结果，使用colorbuffer写入到材质attachment中
    GLuint ssaoFBO;
    glGenFramebuffers(1, &ssaoFBO);  
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    unsigned int ssaoColorBuffer;
    glGenTextures(1,&ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D,ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

    //--------还需要后处理把画面blur一下，因为4x4的噪声会在画面上呈现出非常规律的噪声形状
    unsigned int  ssaoBlurFBO, ssaoColorBufferBlur;
    glGenFramebuffers(1,&ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,ssaoBlurFBO);
    glGenTextures(1,&ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D,ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

    //----------------模型创建-------------------------
    Model croissant(resourcePath("src/models/croissant_4k.gltf/croissant_4k.gltf"));
    Model sponza(resourcePath("src/models/sponza_mcguire/sponza.obj"));

    std::string vertexShaderPath = resourcePath("src/vertexShader/vertexShader.vs");  
    std::string planetVertex = resourcePath("src/vertexShader/planetVertex.vs"); 
    std::string fragmentShaderPath = resourcePath("src/fragmentShader/fragmentShader.fs");
    std::string frameVS = resourcePath("src/vertexShader/frameBufferShader.vs");
    std::string frameFS = resourcePath("src/fragmentShader/frameBufferShader.fs");
    std::string blurFS = resourcePath("src/fragmentShader/blurShader.fs");
    std::string lightFS = resourcePath("src/fragmentShader/lightFragmentShader.fs");
    std::string pointLightVolueFS = resourcePath("src/fragmentShader/pointLightVolueFS.fs");
    std::string toneMapFS = resourcePath("src/fragmentShader/toneMapShader.fs");
    std::string ssaoVS = resourcePath("src/vertexShader/ssao.vs");
    std::string ssaoFS = resourcePath("src/fragmentShader/ssao.fs");
    std::string ssaoBlurFS = resourcePath("src/fragmentShader/ssaoBlur.fs");
    std::string debugRedFS = resourcePath("src/fragmentShader/debugRedChannel.fs");

    Shader sceneShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    Shader fboShader(frameVS.c_str(),frameFS.c_str());
    Shader blurShader(frameVS.c_str(),blurFS.c_str());
    Shader lightShader(vertexShaderPath.c_str(),lightFS.c_str());
    Shader pointLightVolueShader(vertexShaderPath.c_str(),pointLightVolueFS.c_str());
    Shader toneMapShader(frameVS.c_str(),toneMapFS.c_str());
    Shader shaderSSAO(ssaoVS.c_str(),ssaoFS.c_str());
    Shader shaderSSAOBlur(ssaoVS.c_str(),ssaoBlurFS.c_str());
    Shader debugRedShader(frameVS.c_str(),debugRedFS.c_str());

    glm::vec3 sceneClearColor(0.0f,0.0f,0.0f);
    unsigned int woodTexture = TextureFromFile("texture_brick.jpg", resourcePath("src/texture"));

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),width/height,0.1f,100.0f);
    glm::mat4 model = glm::mat4(1.0f);
    float exposure = 0.2;
    float coefficient = 0.3;
    bool useSSAO = true;
    glEnable(GL_DEPTH_TEST);
    initLights();

    while(!glfwWindowShouldClose(window)){
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        
        processInput(window,deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug");
        ImGui::ColorEdit3("Clear Color", glm::value_ptr(sceneClearColor));
        ImGui::DragFloat("exposure",&exposure,0.01f,0.01f,1.0f);
        ImGui::DragFloat("coefficient",&coefficient,0.01f,0.01f,1.0f);
        ImGui::Checkbox("SSAO", &useSSAO);
        ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::End();

        glBindFramebuffer(GL_FRAMEBUFFER,gBuffer);
        //新frame之前清空所有bit
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

        //---------------画场景-----------------
        //我们要移动相机，view就得更新
        glm::mat4 view = camera.GetViewMatrix();

        sceneShader.use();
        sceneShader.setMat4("view", view);
        sceneShader.setMat4("projection", projection);
        sceneShader.setVec3("viewPos", camera.Position);
        glm::mat4 sponzaModel = glm::mat4(1.0f);
        sponzaModel = glm::scale(sponzaModel,glm::vec3(0.01f));
        sceneShader.setMat4("model",sponzaModel);
        sponza.Draw(sceneShader);

        //----------渲染ssao纹理------------
        glBindFramebuffer(GL_FRAMEBUFFER,ssaoFBO);
        shaderSSAO.use();
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        for(unsigned int i = 0; i < 64; i++){
            shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,gPosition);
        shaderSSAO.setInt("gPosition",0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,gNormal);
        shaderSSAO.setInt("gNormal",1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,noiseTexture);
        shaderSSAO.setInt("texNoise",2);
        shaderSSAO.setMat4("projection", projection);
        shaderSSAO.setMat4("view", view);       
        shaderSSAO.setVec2("noiseScale", glm::vec2(width / 4.0f, height / 4.0f));
        drawQuad();

        //-------------做Blur处理---------------
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER,ssaoBlurFBO);
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        shaderSSAOBlur.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,ssaoColorBuffer);
        shaderSSAOBlur.setInt("ssaoInput",0);
        drawQuad();
        
        //----------画画面从fbo（这里是画dirlight照亮的场景，不画点光源先-----------
        glBindFramebuffer(GL_FRAMEBUFFER,hdrFBO);
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        fboShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,gPosition);
        fboShader.setInt("gPosition",0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,gNormal);
        fboShader.setInt("gNormal",1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,gAlbedoSpecular);
        fboShader.setInt("gAlbedoSpecular",2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D,ssaoColorBufferBlur);
        fboShader.setInt("ssaoInput",3);
        fboShader.setVec3("viewPos", camera.Position);
        fboShader.setVec3("dirLight.Direction", -0.2f, -1.0f, -0.3f);
        fboShader.setVec3("dirLight.Ambient", 0.2f, 0.2f, 0.2f);
        fboShader.setVec3("dirLight.Diffuse", 0.15f, 0.15f, 0.15f);
        fboShader.setVec3("dirLight.Specular", 0.2f, 0.2f, 0.2f);
        fboShader.setBool("useSSAO", useSSAO);
        drawQuad();


        
        //------------画光体积-------------
        glCullFace(GL_BACK); // 或 GL_BACK，二选一试
        glEnable(GL_CULL_FACE);
        pointLightVolueShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,gPosition);
        pointLightVolueShader.setInt("gPosition",0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,gNormal);
        pointLightVolueShader.setInt("gNormal",1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,gAlbedoSpecular);
        pointLightVolueShader.setInt("gAlbedoSpecular",2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D,ssaoColorBufferBlur);
        pointLightVolueShader.setInt("ssaoInput",3);
        pointLightVolueShader.setMat4("view",view);
        pointLightVolueShader.setMat4("projection",projection);
        pointLightVolueShader.setVec3("viewPos",camera.Position);
        pointLightVolueShader.setVec2("screenSize",glm::vec2(width,height));
        pointLightVolueShader.setBool("useSSAO", useSSAO);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);
        drawLightsVolume(pointLightVolueShader,coefficient);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        //--------HDR tone mapping to default framebuffer---------
        
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        toneMapShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,hdrColorBuffer);
        toneMapShader.setInt("hdrBuffer",0);
        toneMapShader.setFloat("exposure",exposure);
        drawQuad();

        glEnable(GL_DEPTH_TEST);

        //--------前向渲染灯----------
        glBindFramebuffer(GL_READ_FRAMEBUFFER,gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
        glBlitFramebuffer(0,0,width,height,0,0,width,height,GL_DEPTH_BUFFER_BIT,GL_NEAREST);
        lightShader.use();
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);
        lightShader.setFloat("exposure",exposure);
        drawLights(lightShader);
        

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //双缓冲，交换颜色缓冲
        glfwSwapBuffers(window);
        //检测有无事件触发（比如键盘输入鼠标移动），更新窗口并且调用回调函数
        glfwPollEvents();

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    //释放所有资源
    glfwTerminate();
    return 0;
}


