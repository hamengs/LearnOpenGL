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

struct ObjectMaterial{
    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;
    float Shininess;
};

//窗口大小
float width = 1920;
float height = 1080;

//相机设置全局变量
Camera camera(glm::vec3(0.0f,0.0f,3.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f),45.0f,0.0f,-90.0f);
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

    GLFWwindow* window = glfwCreateWindow(width,height,"LearnOpenGL",NULL,NULL);
    if(window==NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glm::vec3 cubePositions[] = {
      glm::vec3( 0.0f,  0.0f,  0.0f), 
      glm::vec3( 2.0f,  5.0f, -15.0f), 
      glm::vec3(-1.5f, -2.2f, -2.5f),  
      glm::vec3(-3.8f, -2.0f, -12.3f),  
      glm::vec3( 2.4f, -0.4f, -3.5f),  
      glm::vec3(-1.7f,  3.0f, -7.5f),  
      glm::vec3( 1.3f, -2.0f, -2.5f),  
      glm::vec3( 1.5f,  2.0f, -2.5f), 
      glm::vec3( 1.5f,  0.2f, -1.5f), 
      glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    //-----------------草----------------------
    float grassSquareVertices[] = {
        -1.0f, -1.0f, -0.5f,  0.0f, 0.0f,
        1.0f, -1.0f, -0.5f,  1.0f, 0.0f,
        1.0f,  1.0f, -0.5f,  1.0f, 1.0f,
        1.0f,  1.0f, -0.5f,  1.0f, 1.0f,
        -1.0f,  1.0f, -0.5f,  0.0f, 1.0f,
        -1.0f, -1.0f, -0.5f,  0.0f, 0.0f,
    };

    std::vector<glm::vec3> vegetation;
    vegetation.push_back(glm::vec3(-1.5f,  0.0f, -0.48f));
    vegetation.push_back(glm::vec3( 1.5f,  0.0f,  0.51f));
    vegetation.push_back(glm::vec3( 0.0f,  0.0f,  0.7f));
    vegetation.push_back(glm::vec3(-0.3f,  0.0f, -2.3f));
    vegetation.push_back(glm::vec3( 0.5f,  0.0f, -0.6f));

    std::map<float,glm::vec3> sorted;
    for(unsigned int i = 0; i<vegetation.size(); i++){
        float distance = glm::length(camera.Position-vegetation[i]);
        sorted[distance] = vegetation[i];
    }

    //--------------------光---------------------
    glm::vec3 lightPosition(1.2f,1.0f,2.0f);
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };
    glm::mat4 lightModel = glm::mat4(1.0f);

    //------------------Grass Texture--------------------
    Texture grass;
    glGenTextures(1,&grass.id);
    std::string grassPath = resourcePath("src/texture/blending_transparent_window.png");
    int grassWidth,grassHeight,nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *grassData = stbi_load(grassPath.c_str(),&grassWidth,&grassHeight,&nrChannels,0);
    glBindTexture(GL_TEXTURE_2D,grass.id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,grassWidth,grassHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,grassData);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(grassData);

    unsigned int texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int rbo;
    glGenRenderbuffers(1,&rbo);
    glBindRenderbuffer(GL_RENDERBUFFER,rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,width,height);
    


    unsigned int fbo;
    glGenFramebuffers(1,&fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,rbo);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    //创建VAO
    unsigned int VAO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    //创建VBO并且把顶点数据塞入VBO中。
    unsigned int VBO;
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    //解析缓存里的数据，告诉openGL如何解析顶点数据
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightVAO;
    glGenVertexArrays(1,&lightVAO);
    glBindVertexArray(lightVAO);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    //不同的顶点数据，所以VBO也不同

    unsigned int grassVAO;
    glGenVertexArrays(1,&grassVAO);
    glBindVertexArray(grassVAO);
    
    unsigned int grassVBO;
    glGenBuffers(1,&grassVBO);
    glBindBuffer(GL_ARRAY_BUFFER,grassVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(grassSquareVertices),grassSquareVertices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);

    //----------------模型创建-------------------------
    Model croissant(resourcePath("src/models/croissant_4k.gltf/croissant_4k.gltf"));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model,glm::vec3(0.0f,0.0f,-4.0f));
    model = glm::scale(model,glm::vec3(10.0f));
    

    std::string vertexShaderPath = resourcePath("src/vertexShader/vertexShader.vs");
    std::string fragmentShaderPath = resourcePath("src/fragmentShader/fragmentShader.fs");
    std::string lightFragmentShaderPath = resourcePath("src/fragmentShader/lightFragmentShader.fs");
    std::string singleColorShaderPath = resourcePath("src/fragmentShader/singleColorShader.fs");
    std::string grassShaderPath = resourcePath("src/fragmentShader/grassShader.fs");
    std::string FragmentframeBufferShader = resourcePath("src/fragmentShader/frameBufferShader.fs");
    std::string VertexframeBufferShader = resourcePath("src/vertexShader/frameBufferShader.vs");
    Shader myShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    Shader lightShader(vertexShaderPath.c_str(), lightFragmentShaderPath.c_str());
    Shader singleColorShader(vertexShaderPath.c_str(),singleColorShaderPath.c_str());
    Shader grassShader(vertexShaderPath.c_str(),grassShaderPath.c_str());
    Shader frameBufferShader(VertexframeBufferShader.c_str(),FragmentframeBufferShader.c_str());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glm::vec3 sceneClearColor(0.3f, 0.4f, 0.5f);
    float croissantScale = 10.0f;

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
        ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::End();

        glBindFramebuffer(GL_FRAMEBUFFER,fbo);

        //清楚屏幕后用什么颜色代替
        glClearColor(sceneClearColor.r, sceneClearColor.g, sceneClearColor.b, 1.0f);
        //清空颜色缓冲位
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        //设置要用的shader
        myShader.use();
        // Light
        //平行光
        myShader.setVec3("dirLight.Direction", -0.2f, -1.0f, -0.3f);
        myShader.setVec3("dirLight.Ambient",   0.05f, 0.05f, 0.05f);
        myShader.setVec3("dirLight.Diffuse",   0.4f,  0.4f,  0.4f);
        myShader.setVec3("dirLight.Specular",  0.5f,  0.5f,  0.5f);
        //点光源
        for (int i = 0; i < 4; i++) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
        
            myShader.setVec3(base + ".Position", pointLightPositions[i]);
            myShader.setVec3(base + ".Ambient",  0.05f, 0.05f, 0.05f);
            myShader.setVec3(base + ".Diffuse",  0.8f,  0.8f,  0.8f);
            myShader.setVec3(base + ".Specular", 1.0f,  1.0f,  1.0f);
        
            myShader.setFloat(base + ".Constant", 1.0f);
            myShader.setFloat(base + ".Linear",   0.09f);
            myShader.setFloat(base + ".Quadratic", 0.032f);
        }
        //聚光
        myShader.setVec3("spotLight.Position", camera.Position.x, camera.Position.y, camera.Position.z);
        myShader.setVec3("spotLight.Direction", camera.Front.x, camera.Front.y, camera.Front.z);
        myShader.setFloat("spotLight.CutOff",glm::cos(glm::radians(12.5f)));
        myShader.setFloat("spotLight.OuterCutOff",glm::cos(glm::radians(20.0f)));
        myShader.setVec3("spotLight.Ambient",  0.2f, 0.2f, 0.2f);
        myShader.setVec3("spotLight.Diffuse",  0.8f, 0.8f, 0.8f);
        myShader.setVec3("spotLight.Specular", 1.0f, 1.0f, 1.0f);
        //设置相机
        myShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
        myShader.setFloat("material.Shininess", 32.0f);
        
        glBindVertexArray(VAO);
        
        
        glm::mat4 currentModel = glm::mat4(1.0f);
        currentModel = glm::translate(currentModel,glm::vec3(0.0f,0.0f,-4.0f));
        currentModel = glm::scale(currentModel,glm::vec3(croissantScale));
        myShader.setMat4("model", currentModel);
        myShader.setMat4("view",camera.GetViewMatrix());

        //-----------投影矩阵不变，大家都可以用----------------
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Fov),width/height,0.1f,100.0f);
        myShader.setMat4("projection",projection);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS,1,0xFF);
        glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
        glStencilMask(0xFF);

        croissant.Draw(myShader);
        
        lightShader.use();
        lightShader.setMat4("view",camera.GetViewMatrix());
        lightShader.setMat4("projection",projection);
        for(int i = 0; i < 4; i++){
            
            lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel,pointLightPositions[i]);
            lightModel = glm::scale(lightModel,glm::vec3(0.2f,0.2f,0.2f));
            lightShader.setMat4("model", lightModel);
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES,0,36);
        }

        //画高亮
        singleColorShader.use();
        singleColorShader.setMat4("view",camera.GetViewMatrix());
        singleColorShader.setMat4("projection",projection);
        glm::mat4 highLightModel = glm::mat4(1.0f);
        highLightModel = glm::scale(highLightModel,glm::vec3(10.5f));
        singleColorShader.setMat4("model",highLightModel);

        glStencilFunc(GL_NOTEQUAL,1,0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        //croissant.Draw(singleColorShader);

        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        //面剔除，剔除正面，正面定义为顺时针，所以剔除实际背面
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_FRONT);
        // glFrontFace(GL_CW);
        
        //画草
        grassShader.use();
        for(std::map<float,glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it ++){
            glm::mat4 grassModel = glm::mat4(1.0f);
            grassModel = glm::translate(grassModel,it->second);
            grassModel = glm::scale(grassModel,glm::vec3(0.8f));
            grassShader.setMat4("model",grassModel);
            grassShader.setMat4("view",camera.GetViewMatrix());
            grassShader.setMat4("projection",projection);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,grass.id);
            grassShader.setInt("texture1",0);
            glBindVertexArray(grassVAO);
            glDrawArrays(GL_TRIANGLES,0,6);
        }

        
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 返回默认
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        frameBufferShader.use();
        frameBufferShader.setInt("screenTexture",0);
        glBindVertexArray(grassVAO);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D,texture);
        glDrawArrays(GL_TRIANGLES,0,6);
        
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


