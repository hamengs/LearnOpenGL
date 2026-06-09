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
Camera camera(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f),45.0f,0.0f,-90.0f);
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

    stbi_set_flip_vertically_on_load(true);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //---------------------skybox vertices-----------------
    float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

    //--------------------box vertices-----------------
    float cubeVertices[] = {
    // positions          // normals
    // back face
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    // front face
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    // left face
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

    // right face
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    // bottom face
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    // top face
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};
    
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
    
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    bool firstDraw = true;
    unsigned int DEMTexture;
    glGenTextures(1,&DEMTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP,DEMTexture);
    for(int i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

    unsigned int rbo;
    glGenRenderbuffers(1,&rbo);
    glBindRenderbuffer(GL_RENDERBUFFER,rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,512,512);
    
    unsigned int fbo;
    glGenFramebuffers(1,&fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,rbo);


    //--------------------天空盒VAO---------------------
    unsigned int skyboxVAO;
    glGenVertexArrays(1,&skyboxVAO);
    glBindVertexArray(skyboxVAO);
    
    unsigned int skyboxVBO;
    glGenBuffers(1,&skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER,skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(skyboxVertices),skyboxVertices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    //-----------------------箱子VAO-----------------
    unsigned int boxVAO;
    glGenVertexArrays(1,&boxVAO);
    glBindVertexArray(boxVAO);
    
    unsigned int boxVBO;
    glGenBuffers(1,&boxVBO);
    glBindBuffer(GL_ARRAY_BUFFER,boxVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    //----------------模型创建-------------------------
    Model croissant(resourcePath("src/models/croissant_4k.gltf/croissant_4k.gltf"));

    std::string vertexShaderPath = resourcePath("src/vertexShader/vertexShader.vs");
    std::string fragmentShaderPath = resourcePath("src/fragmentShader/fragmentShader.fs");
    std::string FragmentframeBufferShader = resourcePath("src/fragmentShader/frameBufferShader.fs");
    std::string VertexframeBufferShader = resourcePath("src/vertexShader/frameBufferShader.vs");
    std::string vertexSkyboxShader = resourcePath("src/vertexShader/skyboxShader.vs");
    std::string fragmentSkyboxShader = resourcePath("src/fragmentShader/skyboxShader.fs");
    Shader myShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    Shader frameBufferShader(VertexframeBufferShader.c_str(),FragmentframeBufferShader.c_str());
    Shader skyboxShader(vertexSkyboxShader.c_str(),fragmentSkyboxShader.c_str());
    
    //---------------cubemap贴图--------------
    stbi_set_flip_vertically_on_load(false);
    std::vector<std::string> faces
    {
        resourcePath("src/texture/skybox/right.jpg"),
        resourcePath("src/texture/skybox/left.jpg"),
        resourcePath("src/texture/skybox/top.jpg"),
        resourcePath("src/texture/skybox/bottom.jpg"),
        resourcePath("src/texture/skybox/front.jpg"),
        resourcePath("src/texture/skybox/back.jpg")
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glm::vec3 sceneClearColor(0.3f, 0.4f, 0.5f);
    float croissantScale = 10.0f;
    glm::vec3 boxPosition = glm::vec3(0.0f,0.0f,-2.0f);
    glm::mat4 boxModel = glm::mat4(1.0f);
    boxModel = glm::translate(boxModel,boxPosition);
    glm::vec3 capturePos = boxPosition;

    glm::mat4 captureView[] = {
        glm::lookAt(capturePos,capturePos+glm::vec3(1,0,0),glm::vec3(0,-1,0)), //+X
        glm::lookAt(capturePos,capturePos+glm::vec3(-1,0,0),glm::vec3(0,-1,0)), //-X

        glm::lookAt(capturePos,capturePos+glm::vec3(0,1,0),glm::vec3(0,0,1)), //+Y
        glm::lookAt(capturePos,capturePos+glm::vec3(0,-1,0),glm::vec3(0,0,-1)), //-Y

        glm::lookAt(capturePos,capturePos+glm::vec3(0,0,1),glm::vec3(0,-1,0)), //+Z
        glm::lookAt(capturePos,capturePos+glm::vec3(0,0,-1),glm::vec3(0,-1,0)), //-Z

    };

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
        if(firstDraw){
            glBindFramebuffer(GL_FRAMEBUFFER,fbo);
            glViewport(0,0,512,512);
            for(int i = 0; i < 6; i++){

                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,DEMTexture,0);
                if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
                    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
                }
                //清楚屏幕后用什么颜色代替
                glClearColor(sceneClearColor.r, sceneClearColor.g, sceneClearColor.b, 1.0f);
                //清空颜色缓冲位
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);

                //-----------投影矩阵不变---------------
                glm::mat4 projection;
                projection = glm::perspective(glm::radians(90.0f),1.0f,0.1f,100.0f);
                glm::mat4 view = captureView[i];
                //----------画skybox------------
                glDepthMask(GL_FALSE);
                skyboxShader.use();
                skyboxShader.setMat4("view",glm::mat3(view));
                skyboxShader.setMat4("projection",projection);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP,cubemapTexture);
                skyboxShader.setInt("skybox",0);
                glBindVertexArray(skyboxVAO);
                glDrawArrays(GL_TRIANGLES,0,36);
                glDepthMask(GL_TRUE);

                //设置要用的shader
                myShader.use();
                // --------------------Light-------------------------
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
                glm::mat4 currentModel = glm::mat4(1.0f);
                currentModel = glm::translate(currentModel,glm::vec3(0.0f,0.0f,-4.0f));
                currentModel = glm::scale(currentModel,glm::vec3(croissantScale));
                myShader.setMat4("model", currentModel);
                myShader.setMat4("view",view);  

                myShader.setMat4("projection",projection);
                croissant.Draw(myShader);
            }
            firstDraw = false;
        }
        
        //先画到fbo里，然后才能画箱子
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glClearColor(sceneClearColor.r, sceneClearColor.g, sceneClearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0,0,width,height);
        //--------------正常画一遍场景------------------
        //-----------投影矩阵不变---------------
        glm::mat4 projectionReal = glm::perspective(glm::radians(90.0f),width/height,0.1f,100.0f);
        glm::mat4 viewReal = camera.GetViewMatrix();
        //----------画skybox------------
        glDepthMask(GL_FALSE);
        skyboxShader.use();
        skyboxShader.setMat4("view",glm::mat3(viewReal));
        skyboxShader.setMat4("projection",projectionReal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP,cubemapTexture);
        skyboxShader.setInt("skybox",0);
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES,0,36);
        glDepthMask(GL_TRUE);

        //设置要用的shader
        myShader.use();
        // --------------------Light-------------------------
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
        glm::mat4 currentModel = glm::mat4(1.0f);
        currentModel = glm::translate(currentModel,glm::vec3(0.0f,0.0f,-4.0f));
        currentModel = glm::scale(currentModel,glm::vec3(croissantScale));
        myShader.setMat4("model", currentModel);
        myShader.setMat4("view",viewReal);  

        myShader.setMat4("projection",projectionReal);
        croissant.Draw(myShader);

        //---------------画箱子-----------------
        
        frameBufferShader.use();
        glm::mat4 modelBox = glm::mat4(1.0f);
        modelBox = glm::translate(modelBox,glm::vec3(0,0,-2.0f));
        modelBox = glm::scale(modelBox,glm::vec3(0.5f));
        frameBufferShader.setMat4("model",modelBox);
        frameBufferShader.setMat4("view",viewReal);
        frameBufferShader.setMat4("projection",projectionReal);
        frameBufferShader.setVec3("cameraPos",camera.Position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP,DEMTexture);
        frameBufferShader.setInt("screenTexture",0);
        glBindVertexArray(boxVAO);
        glDrawArrays(GL_TRIANGLES,0,36);
        
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


