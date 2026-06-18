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
glm::vec3 lightPositions[] = {
    glm::vec3(0.0f, 1.5f, 1.5f),
    glm::vec3(2.8f, 0.4f, -1.6f)
};
glm::vec3 lightColors[] = {
    glm::vec3(4.0f, 3.2f, 1.8f),
    glm::vec3(1.2f, 2.2f, 4.0f)
};

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
    for(int i = 0; i < 2; i++){
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
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,width,height,0,GL_RGB,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,gPosition,0);

    //--------gNormal------------
    unsigned int gNormal;
    glGenTextures(1,&gNormal);
    glBindTexture(GL_TEXTURE_2D,gNormal);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,width,height,0,GL_RGB,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,gNormal,0);

    //--------gAlbedoSpecular------------
    unsigned int gAlbedoSpecular;
    glGenTextures(1,&gAlbedoSpecular);
    glBindTexture(GL_TEXTURE_2D,gAlbedoSpecular);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
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


    //----------------模型创建-------------------------
    Model croissant(resourcePath("src/models/croissant_4k.gltf/croissant_4k.gltf"));

    std::string vertexShaderPath = resourcePath("src/vertexShader/vertexShader.vs");  
    std::string planetVertex = resourcePath("src/vertexShader/planetVertex.vs"); 
    std::string fragmentShaderPath = resourcePath("src/fragmentShader/fragmentShader.fs");
    std::string frameVS = resourcePath("src/vertexShader/frameBufferShader.vs");
    std::string frameFS = resourcePath("src/fragmentShader/frameBufferShader.fs");
    std::string blurFS = resourcePath("src/fragmentShader/blurShader.fs");
    std::string lightFS = resourcePath("src/fragmentShader/lightFragmentShader.fs");

    Shader sceneShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    Shader fboShader(frameVS.c_str(),frameFS.c_str());
    Shader blurShader(frameVS.c_str(),blurFS.c_str());
    Shader lightShader(vertexShaderPath.c_str(),lightFS.c_str());
    glm::vec3 sceneClearColor(0.0f,0.0f,0.0f);
    unsigned int woodTexture = TextureFromFile("texture_brick.jpg", resourcePath("src/texture"));

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),width/height,0.1f,1000.0f);
    glm::mat4 model = glm::mat4(1.0f);
    float exposure = 0.2;
    glEnable(GL_DEPTH_TEST);

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
        ImGui::DragFloat("exposure",&exposure,0.02f,0.1f,1.0f);
        ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::End();

        glBindFramebuffer(GL_FRAMEBUFFER,gBuffer);
        //新frame之前清空所有bit
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

        //我们要移动相机，view就得更新
        glm::mat4 view = camera.GetViewMatrix();
        sceneShader.use();
        sceneShader.setMat4("view", view);
        sceneShader.setMat4("projection", projection);
        sceneShader.setVec3("viewPos", camera.Position);
        sceneShader.setFloat("material.Shininess", 32.0f);
        sceneShader.setInt("material.Diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        drawBoxes(sceneShader);
        
        
        //----------画画面从fbo-----------

        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
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
        fboShader.setVec3("viewPos", camera.Position);
        fboShader.setVec3("dirLight.Direction", -0.2f, -1.0f, -0.3f);
        fboShader.setVec3("dirLight.Ambient", 0.02f, 0.02f, 0.02f);
        fboShader.setVec3("dirLight.Diffuse", 0.15f, 0.15f, 0.15f);
        fboShader.setVec3("dirLight.Specular", 0.2f, 0.2f, 0.2f);
        for(int i = 0; i < 2; i++){
            std::string index = std::to_string(i);
            fboShader.setVec3("pointLights[" + index + "].Position", lightPositions[i]);
            fboShader.setVec3("pointLights[" + index + "].Ambient", lightColors[i] * 0.02f);
            fboShader.setVec3("pointLights[" + index + "].Diffuse", lightColors[i]);
            fboShader.setVec3("pointLights[" + index + "].Specular", lightColors[i]);
            fboShader.setFloat("pointLights[" + index + "].Constant", 1.0f);
            fboShader.setFloat("pointLights[" + index + "].Linear", 0.09f);
            fboShader.setFloat("pointLights[" + index + "].Quadratic", 0.032f);
        }
        fboShader.setFloat("exposure",exposure);
        drawQuad();
        glEnable(GL_DEPTH_TEST);
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


