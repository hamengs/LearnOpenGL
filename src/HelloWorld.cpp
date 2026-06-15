#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//窗口大小
float width = 800;
float height = 600;

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
    if(nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;
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

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
unsigned int planeVAO = 0;
unsigned int planeVBO = 0;

void renderCube()
{
    if (cubeVAO == 0) {
        // Positions are written counter-clockwise as viewed from the outside of each cube face.
        float vertices[] = {
            // positions            // normals           // texcoords
            // back face
            -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
            // front face
            -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
            // left face
            -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
            // right face
             0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
            // bottom face
            -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
            // top face
            -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 0.0f
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

void renderPlane()
{
    if (planeVAO == 0) {
        // Local plane lies on XZ, normal points to +Y. Vertices are CCW when viewed from +Y.
        float vertices[] = {
            // positions            // normals           // texcoords
            -0.5f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
            -0.5f, 0.0f,  0.5f,     0.0f, 1.0f, 0.0f,    0.0f, 4.0f,
             0.5f, 0.0f,  0.5f,     0.0f, 1.0f, 0.0f,    4.0f, 4.0f,
             0.5f, 0.0f,  0.5f,     0.0f, 1.0f, 0.0f,    4.0f, 4.0f,
             0.5f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,    4.0f, 0.0f,
            -0.5f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,    0.0f, 0.0f
        };

        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(planeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void renderScene(Shader& shader, unsigned int roomTexture, unsigned int boxTexture)
{
    glm::mat4 model = glm::mat4(1.0f);

    glBindTexture(GL_TEXTURE_2D, roomTexture);

    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(16.0f, 1.0f, 16.0f));
    shader.setMat4("model", model);
    renderPlane();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 7.0f, 0.0f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(16.0f, 1.0f, 16.0f));
    shader.setMat4("model", model);
    renderPlane();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 3.0f, -7.5f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(16.0f, 1.0f, 8.0f));
    shader.setMat4("model", model);
    renderPlane();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 3.0f, 7.5f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(16.0f, 1.0f, 8.0f));
    shader.setMat4("model", model);
    renderPlane();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-7.5f, 3.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(8.0f, 1.0f, 16.0f));
    shader.setMat4("model", model);
    renderPlane();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(7.5f, 3.0f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(8.0f, 1.0f, 16.0f));
    shader.setMat4("model", model);
    renderPlane();

    glBindTexture(GL_TEXTURE_2D, boxTexture);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, -0.15f, -1.0f));
    model = glm::rotate(model, glm::radians(18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.6f, 1.7f, 1.6f));
    shader.setMat4("model", model);
    renderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.2f, -0.45f, 1.7f));
    model = glm::rotate(model, glm::radians(-24.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.5f, 1.1f, 2.0f));
    shader.setMat4("model", model);
    renderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.2f, 0.55f, -3.8f));
    model = glm::rotate(model, glm::radians(32.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 3.0f, 1.0f));
    shader.setMat4("model", model);
    renderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.2f, 5.7f, 0.8f));
    model = glm::rotate(model, glm::radians(-18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.2f, 0.8f, 1.2f));
    shader.setMat4("model", model);
    renderCube();
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

    //-----------创建cubemap--------------
    const int SHADOW_WIDTH = 1024;
    const int SHADOW_HEIGHT = 1024;
    unsigned int depthCubemap;
    glGenTextures(1,&depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP,depthCubemap);
    for(int i = 0; i<6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_DEPTH_COMPONENT,SHADOW_WIDTH,SHADOW_HEIGHT,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

    //----------------创建shadowFBO-----------
    unsigned int depthMapFBO;
    glGenFramebuffers(1,&depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,depthCubemap,0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    //----------------模型创建-------------------------
    Model croissant(resourcePath("src/models/croissant_4k.gltf/croissant_4k.gltf"));

    std::string vertexShaderPath = resourcePath("src/vertexShader/vertexShader.vs");  
    std::string planetVertex = resourcePath("src/vertexShader/planetVertex.vs"); 
    std::string fragmentShaderPath = resourcePath("src/fragmentShader/fragmentShader.fs");
    std::string frameVertex = resourcePath("src/vertexShader/frameBufferShader.vs");
    std::string frameFrag = resourcePath("src/fragmentShader/frameBufferShader.fs");
    std::string frameGeo = resourcePath("src/geometryShader/geometryShader.gs");
    
    Shader myShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    Shader shadowMapShader(frameVertex.c_str(), frameFrag.c_str(),frameGeo.c_str());
    glm::vec3 sceneClearColor(0.0f,0.0f,0.0f);
    glm::vec3 pointLightPos(0.0f, 3.0f, 0.0f);
    unsigned int roomTexture = TextureFromFile("texture_brick.jpg", resourcePath("src/texture"));
    unsigned int boxTexture = TextureFromFile("container2.png", resourcePath("src/texture"));

    //--------------牛角包设置--------------
    float croissantScale = 8.0f;
    glm::mat4 croissantModel = glm::mat4(1.0f);
    croissantModel = glm::scale(croissantModel,glm::vec3(croissantScale));
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),width/height,0.1f,1000.0f);
    glm::mat4 model = glm::mat4(1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDepthFunc(GL_LESS);

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
        ImGui::SliderFloat("Croissant Scale", &croissantScale, 2.0f, 16.0f);
        ImGui::SliderFloat3("Point Light", glm::value_ptr(pointLightPos), -5.0f, 5.0f);
        ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::End();

        //-------------先渲染到depthmap里------------
        glViewport(0,0,SHADOW_WIDTH,SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO);
        //新frame之前清空所有bit
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

        //----------------创建6个面的lookat和perspective(每帧都要重新创建，因为imgui)---------------
        std::vector<glm::mat4> lookAtMatrices;
        glm::mat4 depthMapPerspective = glm::perspective(glm::radians(90.0f),(float)SHADOW_WIDTH/SHADOW_HEIGHT,0.1f,20.0f);
        lookAtMatrices.push_back(depthMapPerspective*glm::lookAt(pointLightPos,pointLightPos+glm::vec3(1,0,0),glm::vec3(0,-1,0)));
        lookAtMatrices.push_back(depthMapPerspective*glm::lookAt(pointLightPos,pointLightPos+glm::vec3(-1,0,0),glm::vec3(0,-1,0)));
        lookAtMatrices.push_back(depthMapPerspective*glm::lookAt(pointLightPos,pointLightPos+glm::vec3(0,1,0),glm::vec3(0,0,1)));
        lookAtMatrices.push_back(depthMapPerspective*glm::lookAt(pointLightPos,pointLightPos+glm::vec3(0,-1,0),glm::vec3(0,0,-1)));
        lookAtMatrices.push_back(depthMapPerspective*glm::lookAt(pointLightPos,pointLightPos+glm::vec3(0,0,1),glm::vec3(0,-1,0)));
        lookAtMatrices.push_back(depthMapPerspective*glm::lookAt(pointLightPos,pointLightPos+glm::vec3(0,0,-1),glm::vec3(0,-1,0)));

        shadowMapShader.use();
        for(int i=0; i<6;i++){
            shadowMapShader.setMat4("shadowMatrices["+std::to_string(i)+"]",lookAtMatrices[i]);
        }
        shadowMapShader.setFloat("far_plane",20.0f);
        shadowMapShader.setVec3("lightPos",pointLightPos);
        renderScene(shadowMapShader,roomTexture,boxTexture);
        glViewport(0,0,width,height);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        //我们要移动相机，view就得更新
        glm::mat4 view = camera.GetViewMatrix();
        croissantModel = glm::mat4(1.0f);
        croissantModel = glm::scale(croissantModel,glm::vec3(croissantScale));
        myShader.use();
        myShader.setMat4("view", view);
        myShader.setMat4("projection", projection);
        myShader.setVec3("viewPos", camera.Position);
        myShader.setInt("material.Diffuse", 0);
        myShader.setFloat("material.Shininess", 32.0f);
        myShader.setVec3("pointLight.Position", pointLightPos);
        myShader.setVec3("pointLight.Ambient", 0.35f, 0.3f, 0.3f);
        myShader.setVec3("pointLight.Diffuse", 1.0f, 0.82f, 0.55f);
        myShader.setVec3("pointLight.Specular", 1.0f, 0.9f, 0.75f);
        myShader.setFloat("pointLight.Constant", 1.0f);
        myShader.setFloat("pointLight.Linear", 0.09f);
        myShader.setFloat("pointLight.Quadratic", 0.032f);
        myShader.setVec3("lightPos",pointLightPos);
        myShader.setFloat("far_plane",20.0f);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP,depthCubemap);
        myShader.setInt("depthMap",1);
        glActiveTexture(GL_TEXTURE0);
        renderScene(myShader, roomTexture, boxTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPos);
        model = glm::scale(model, glm::vec3(0.18f));
        myShader.setMat4("model", model);
        renderCube();

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


