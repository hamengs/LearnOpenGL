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

void Drawsquare(const std::vector<float>& vertices, Shader& shader, unsigned int texture, unsigned int normalTexture){
    //-------------加载平面-------------------
    unsigned int squareVAO;
    glGenVertexArrays(1,&squareVAO);
    glBindVertexArray(squareVAO);

    unsigned int squareVBO;
    glGenBuffers(1,&squareVBO);
    glBindBuffer(GL_ARRAY_BUFFER,squareVBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture);
    shader.setInt("material.Diffuse",0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,normalTexture);
    shader.setInt("brickNormalMap",1);

    glDrawArrays(GL_TRIANGLES,0,6);
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

        // positions          // normals        // texCoords
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
    
         0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f
    };

    glm::vec3 pointLightPosition = glm::vec3(1.0f,2.0f,0.5f);
    //-------------加载法线贴图--------------
    unsigned int brickNormalMap = TextureFromFile(std::string("brickwall_normal.jpg").c_str(),resourcePath("src/texture"));
    //-------------加载纹理贴图--------------
    unsigned int brickTexture = TextureFromFile(std::string("brickwall.jpg").c_str(),resourcePath("src/texture"));

    //----------------模型创建-------------------------
    Model croissant(resourcePath("src/models/croissant_4k.gltf/croissant_4k.gltf"));

    std::string vertexShaderPath = resourcePath("src/vertexShader/vertexShader.vs");  
    std::string fragmentShaderPath = resourcePath("src/fragmentShader/fragmentShader.fs");

    Shader sceneShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    glm::vec3 sceneClearColor(0.0f,0.0f,0.0f);

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
        ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::DragFloat3("PointLightPos",glm::value_ptr(pointLightPosition),0.05f);
        ImGui::End();

        //新frame之前清空所有bit
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

        //我们要移动相机，view就得更新
        glm::mat4 view = camera.GetViewMatrix();
        sceneShader.use();
        sceneShader.setMat4("view", view);
        sceneShader.setMat4("projection", projection);
        sceneShader.setMat4("model",model);
        sceneShader.setVec3("viewPos",camera.Position);
        sceneShader.setFloat("material.Shininess",32.0f);
        sceneShader.setVec3("pointLight.Position",pointLightPosition);
        sceneShader.setVec3("pointLight.Ambient",  glm::vec3(0.05f));
        sceneShader.setVec3("pointLight.Diffuse",  glm::vec3(0.8f));
        sceneShader.setVec3("pointLight.Specular", glm::vec3(1.0f));
        sceneShader.setFloat("pointLight.Constant",  1.0f);
        sceneShader.setFloat("pointLight.Linear",    0.09f);
        sceneShader.setFloat("pointLight.Quadratic", 0.032f);

        Drawsquare(vertices,sceneShader,brickTexture,brickNormalMap);

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


