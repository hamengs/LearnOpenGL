#include <glad/glad.h> 
#include <glfw3.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"

struct Vertex{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture{
    unsigned int id;
    std::string type;
};

struct ObjectMaterial{
    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;
    float Shininess;
};

//相机设置全局变量
Camera camera(glm::vec3(0.0f,0.0f,3.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f),45.0f,0.0f,-90.0f);
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
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
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
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

    GLFWwindow* window = glfwCreateWindow(800,600,"LearnOpenGL",NULL,NULL);
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
    glViewport(0,0,800,600);

    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window,mouse_callback);
    glfwSetScrollCallback(window,scroll_callback);

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

    ObjectMaterial materials[] = {
        {glm::vec3(1.0f, 0.5f, 0.31f), glm::vec3(1.0f, 0.5f, 0.31f), glm::vec3(0.5f, 0.5f, 0.5f), 32.0f},
        {glm::vec3(0.1f, 0.18f, 0.3f), glm::vec3(0.0f, 0.45f, 0.9f), glm::vec3(0.8f, 0.9f, 1.0f), 96.0f},
        {glm::vec3(0.25f, 0.18f, 0.08f), glm::vec3(0.95f, 0.65f, 0.2f), glm::vec3(1.0f, 0.86f, 0.45f), 64.0f},
        {glm::vec3(0.08f, 0.22f, 0.12f), glm::vec3(0.1f, 0.65f, 0.28f), glm::vec3(0.2f, 0.35f, 0.2f), 12.0f},
        {glm::vec3(0.22f, 0.08f, 0.1f), glm::vec3(0.8f, 0.1f, 0.18f), glm::vec3(0.9f, 0.25f, 0.3f), 24.0f},
        {glm::vec3(0.18f, 0.16f, 0.22f), glm::vec3(0.48f, 0.38f, 0.82f), glm::vec3(0.9f, 0.85f, 1.0f), 128.0f},
        {glm::vec3(0.22f, 0.2f, 0.18f), glm::vec3(0.55f, 0.52f, 0.48f), glm::vec3(0.15f, 0.15f, 0.15f), 8.0f},
        {glm::vec3(0.05f, 0.18f, 0.18f), glm::vec3(0.0f, 0.8f, 0.78f), glm::vec3(0.65f, 1.0f, 0.95f), 48.0f},
        {glm::vec3(0.28f, 0.12f, 0.25f), glm::vec3(0.95f, 0.22f, 0.75f), glm::vec3(1.0f, 0.7f, 0.95f), 80.0f},
        {glm::vec3(0.16f, 0.2f, 0.08f), glm::vec3(0.55f, 0.72f, 0.16f), glm::vec3(0.5f, 0.6f, 0.25f), 20.0f}
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

    glm::vec3 lightDirection(1.2f,1.0f,2.0f);
    glm::vec3 lightPosition(1.2f,1.0f,2.0f);
    glm::mat4 lightModel = glm::mat4(1.0f);
    //lightModel = glm::translate(lightModel,lightPos);
    //lightModel = glm::scale(lightModel,glm::vec3(0.2f,0.2f,0.2f));
    
    //加载纹理
    Texture texture0;
    glGenTextures(1,&texture0.id);
    int width1,height1,nrChannels1;
    unsigned char* data1 = stbi_load(resourcePath("src/texture/container2.png").c_str(),&width1,&height1,&nrChannels1,0);
    glBindTexture(GL_TEXTURE_2D,texture0.id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    GLenum format1 = GL_RGB;
    if(nrChannels1 == 3) format1 = GL_RGB;
    else if (nrChannels1 == 4) format1 = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D,0,format1,width1,height1,0,format1,GL_UNSIGNED_BYTE,data1);
    glGenerateMipmap(GL_TEXTURE_2D);
    if(data1==NULL){
        std::cout<<"text not loaded"<<std::endl;
    }
    stbi_image_free(data1);

    Texture texture1;
    glGenTextures(1,&texture1.id);
    int width2,height2,nrChannels2;
    unsigned char* data2 = stbi_load(resourcePath("src/texture/container2_specular.png").c_str(),&width2,&height2,&nrChannels2,0);
    glBindTexture(GL_TEXTURE_2D,texture1.id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    GLenum format2 = GL_RGB;
    if(nrChannels2 == 3) format2 = GL_RGB;
    else if (nrChannels2 == 4) format2 = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D,0,format2,width2,height2,0,format2,GL_UNSIGNED_BYTE,data2);
    glGenerateMipmap(GL_TEXTURE_2D);
    if(data2==NULL){
        std::cout<<"text not loaded"<<std::endl;
    }
    stbi_image_free(data2);

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

    std::string vertexShaderPath = resourcePath("src/vertexShader/vertexShader.vs");
    std::string fragmentShaderPath = resourcePath("src/fragmentShader/fragmentShader.fs");
    std::string lightFragmentShaderPath = resourcePath("src/fragmentShader/lightFragmentShader.fs");
    Shader myShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    Shader lightShader(vertexShaderPath.c_str(), lightFragmentShaderPath.c_str());

    glEnable(GL_DEPTH_TEST);


    while(!glfwWindowShouldClose(window)){
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        
        processInput(window,deltaTime);

        //清楚屏幕后用什么颜色代替
        glClearColor(0.3f,0.4f,0.5f,1.0f);
        //清空颜色缓冲位
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        //设置要用的shader
        myShader.use();
        //绑定纹理到shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,texture0.id);
        myShader.setInt("material.Diffuse",0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,texture1.id);
        myShader.setInt("material.Specular",1);
        // Light
        myShader.setVec3("light.Position", camera.Position.x, camera.Position.y, camera.Position.z);
        myShader.setVec3("light.Direction", camera.Front.x, camera.Front.y, camera.Front.z);
        myShader.setFloat("light.CutOff",glm::cos(glm::radians(12.5f)));
        myShader.setVec3("light.Ambient",  0.2f, 0.2f, 0.2f);
        myShader.setVec3("light.Diffuse",  0.8f, 0.8f, 0.8f);
        myShader.setVec3("light.Specular", 1.0f, 1.0f, 1.0f);
        myShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
        
        glBindVertexArray(VAO);

        myShader.setMat4("view",camera.GetViewMatrix());

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Fov),4.0f/3.0f,0.1f,100.0f);
        myShader.setMat4("projection",projection);

        //---画三角形--- 画10个
        unsigned int objectCount = sizeof(cubePositions) / sizeof(cubePositions[0]);
        for(unsigned int i = 0; i < objectCount ; i++){
            ObjectMaterial material = materials[i];
            myShader.setVec3("material.Ambient", material.Ambient.x, material.Ambient.y, material.Ambient.z);
            myShader.setVec3("material.Diffuse", material.Diffuse.x, material.Diffuse.y, material.Diffuse.z);
            myShader.setVec3("material.Specular", material.Specular.x, material.Specular.y, material.Specular.z);
            myShader.setFloat("material.Shininess", material.Shininess);
            //使用model之前更新
            glm::mat4 model = glm::mat4(1.0f); //model得在这里初始化，每次都得初始化，不然会叠加
            model = glm::translate(model,cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model,glm::radians(angle),glm::vec3(1.0f,0.3f,0.5f));
            //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
            myShader.setMat4("model",model);
            glDrawArrays(GL_TRIANGLES,0,36);
        }
        
        lightShader.use();
        lightShader.setMat4("model", lightModel);
        lightShader.setMat4("view",camera.GetViewMatrix());
        lightShader.setMat4("projection",projection);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES,0,36);


        //双缓冲，交换颜色缓冲
        glfwSwapBuffers(window);
        //检测有无事件触发（比如键盘输入鼠标移动），更新窗口并且调用回调函数
        glfwPollEvents();

    }

    //释放所有资源
    glfwTerminate();
    return 0;
}


