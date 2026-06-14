#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <vector>
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

    //------------squad--------------
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, -1.0f,  0.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 1.0f,

        -1.0f,  -1.0f,  0.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        1.0f,  -1.0f,  1.0f, 0.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    float planeVertices[] = {
        // positions          // normals        // texCoords
         10.0f, -1.5f,  10.0f, 0.0f, 1.0f, 0.0f, 10.0f,  0.0f,
        -10.0f, -1.5f, -10.0f, 0.0f, 1.0f, 0.0f,  0.0f, 10.0f,
        -10.0f, -1.5f,  10.0f, 0.0f, 1.0f, 0.0f,  0.0f,  0.0f,

         10.0f, -1.5f,  10.0f, 0.0f, 1.0f, 0.0f, 10.0f,  0.0f,
         10.0f, -1.5f, -10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 10.0f,
        -10.0f, -1.5f, -10.0f, 0.0f, 1.0f, 0.0f,  0.0f, 10.0f,
    };

    float cubeVertices[] = {
        // positions          // normals           // texCoords
        -0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f, 0.0f,0.0f,
         0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f, 1.0f,1.0f,
         0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f, 1.0f,0.0f,
         0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f, 1.0f,1.0f,
        -0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f, 0.0f,0.0f,
        -0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f, 0.0f,1.0f,

        -0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,0.0f,
         0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f,0.0f,
         0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f,1.0f,
         0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f,1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,1.0f,
        -0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,0.0f,

        -0.5f, 0.5f, 0.5f,-1.0f, 0.0f, 0.0f, 1.0f,0.0f,
        -0.5f, 0.5f,-0.5f,-1.0f, 0.0f, 0.0f, 1.0f,1.0f,
        -0.5f,-0.5f,-0.5f,-1.0f, 0.0f, 0.0f, 0.0f,1.0f,
        -0.5f,-0.5f,-0.5f,-1.0f, 0.0f, 0.0f, 0.0f,1.0f,
        -0.5f,-0.5f, 0.5f,-1.0f, 0.0f, 0.0f, 0.0f,0.0f,
        -0.5f, 0.5f, 0.5f,-1.0f, 0.0f, 0.0f, 1.0f,0.0f,

         0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f,0.0f,
         0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f, 0.0f,1.0f,
         0.5f, 0.5f,-0.5f, 1.0f, 0.0f, 0.0f, 1.0f,1.0f,
         0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f, 0.0f,1.0f,
         0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f,0.0f,
         0.5f,-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f,0.0f,

        -0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f, 0.0f,1.0f,
         0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f, 1.0f,1.0f,
         0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f, 1.0f,0.0f,
         0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f, 1.0f,0.0f,
        -0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f, 0.0f,0.0f,
        -0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f, 0.0f,1.0f,

        -0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f, 0.0f,1.0f,
         0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f,0.0f,
         0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f, 1.0f,1.0f,
         0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f,0.0f,
        -0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f, 0.0f,1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f
    };

    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    //---------Light--------------
    glm::vec3 lightDir = glm::vec3(-0.2f,-1.0f,-0.3f);
    glm::vec3 lightPos = glm::vec3(6.0f,8.0f,6.0f);
    glm::mat4 lightView = glm::lookAt(lightPos,glm::vec3(0.0f),glm::vec3(0,1,0));
    glm::mat4 lightProjection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,1.0f,20.0f);
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    //-----------SHADOWMAP------------------

    int SHADOW_MAP_WIDTH = 1024;
    int SHADOW_MAP_HEIGHT = 1024;

    unsigned int depthMapFBO;
    glGenFramebuffers(1,&depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO);

    unsigned int depthMapTexture;
    glGenTextures(1,&depthMapTexture);
    glBindTexture(GL_TEXTURE_2D,depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,SHADOW_MAP_WIDTH,SHADOW_MAP_HEIGHT,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depthMapTexture,0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    //---------uniform buffer create---------------
    unsigned int ubo;
    glGenBuffers(1,&ubo);
    glBindBuffer(GL_UNIFORM_BUFFER,ubo);
    glBufferData(GL_UNIFORM_BUFFER,128,NULL,GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER,0);
    

    //----------------模型创建-------------------------
    Model croissant(resourcePath("src/models/croissant_4k.gltf/croissant_4k.gltf"));

    std::string vertexShaderPath = resourcePath("src/vertexShader/vertexShader.vs");  
    std::string planetVertex = resourcePath("src/vertexShader/planetVertex.vs"); 
    std::string fragmentShaderPath = resourcePath("src/fragmentShader/fragmentShader.fs");
    std::string frameVertex = resourcePath("src/vertexShader/frameBufferShader.vs");
    std::string frameFrag = resourcePath("src/fragmentShader/frameBufferShader.fs");

    Shader myShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    Shader shadowMapShader(frameVertex.c_str(),frameFrag.c_str());
    glm::vec3 sceneClearColor(0.0f,0.0f,0.0f);
    unsigned int floorTexture = TextureFromFile("texture_brick.jpg", resourcePath("src/texture"));
    unsigned int cubeTexture = TextureFromFile("container2.png", resourcePath("src/texture"));

    //---------uniformBufferBind-----------------
    unsigned int shadowFrame_index = glGetUniformBlockIndex(shadowMapShader.ID,"Matrices");
    glUniformBlockBinding(shadowMapShader.ID,shadowFrame_index,0);
    glBindBufferBase(GL_UNIFORM_BUFFER,0,ubo);
    glBindBuffer(GL_UNIFORM_BUFFER,ubo);
    glBufferSubData(GL_UNIFORM_BUFFER,0,64,glm::value_ptr(lightView));
    glBufferSubData(GL_UNIFORM_BUFFER,sizeof(glm::mat4),sizeof(glm::mat4),glm::value_ptr(lightProjection));
    glBindBuffer(GL_UNIFORM_BUFFER,0);


    //--------------牛角包设置--------------
    float croissantScale = 8.0f;
    glm::mat4 croissantModel = glm::mat4(1.0f);
    croissantModel = glm::scale(croissantModel,glm::vec3(croissantScale));
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),width/height,0.1f,100.0f);
    glm::mat4 model = glm::mat4(1.0f);

    auto drawScene = [&](Shader &shader){
        shader.setInt("material.Diffuse", 0);
        shader.setFloat("material.Shininess", 32.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glm::mat4 floorModel = glm::mat4(1.0f);
        shader.setMat4("model", floorModel);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        glBindVertexArray(cubeVAO);

        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(-3.0f, -0.5f, -1.5f));
        cubeModel = glm::scale(cubeModel, glm::vec3(2.0f));
        shader.setMat4("model", cubeModel);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(2.0f, -0.8f, 1.0f));
        cubeModel = glm::rotate(cubeModel, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        cubeModel = glm::scale(cubeModel, glm::vec3(1.4f));
        shader.setMat4("model", cubeModel);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(0.5f, 0.0f, -3.0f));
        cubeModel = glm::rotate(cubeModel, glm::radians(-18.0f), glm::vec3(1.0f, 0.0f, 1.0f));
        cubeModel = glm::scale(cubeModel, glm::vec3(1.0f));
        shader.setMat4("model", cubeModel);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
    };

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_CULL_FACE);
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
        ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::End();
        glCullFace(GL_FRONT);

        glViewport(0,0,SHADOW_MAP_WIDTH,SHADOW_MAP_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO);

        //新frame之前清空所有bit
        glClearColor(sceneClearColor.r,sceneClearColor.g,sceneClearColor.b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO);


        //我们要移动相机，view就得更新
        glm::mat4 view = camera.GetViewMatrix();
        croissantModel = glm::mat4(1.0f);
        croissantModel = glm::scale(croissantModel,glm::vec3(croissantScale));
        shadowMapShader.use();
        shadowMapShader.setMat4("lightMatrices",lightSpaceMatrix);
        shadowMapShader.setMat4("model",croissantModel);
        croissant.Draw(shadowMapShader);
        drawScene(shadowMapShader);
        glCullFace(GL_BACK); 

        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glViewport(0,0,width,height);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        myShader.use();
        myShader.setMat4("projection", projection);
        myShader.setMat4("view",view);
        myShader.setMat4("lightMatrices",lightSpaceMatrix);
        myShader.setVec3("dirLight.Direction",lightDir);
        myShader.setVec3("dirLight.Ambient", 0.15f, 0.15f, 0.15f);
        myShader.setVec3("dirLight.Diffuse", 0.8f, 0.8f, 0.8f);
        myShader.setVec3("dirLight.Specular", 1.0f, 1.0f, 1.0f);
        myShader.setVec3("viewPos",camera.Position);


        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,depthMapTexture);
        myShader.setInt("shadowMapTexture",1);
        croissant.Draw(myShader);
        drawScene(myShader);

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


