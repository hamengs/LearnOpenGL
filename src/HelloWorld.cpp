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

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    glm::vec3 lightPos(1.2f,1.0f,2.0f);
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel,lightPos);
    lightModel = glm::scale(lightModel,glm::vec3(0.5f));

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
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int lightVAO;
    glGenVertexArrays(1,&lightVAO);
    glBindVertexArray(lightVAO);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
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
        myShader.setVec3("objectColor",1.0f,0.5f,0.31f);
        myShader.setVec3("lightColor",1.0f,1.0f,1.0f);
        myShader.setVec3("lightPos",lightPos.x,lightPos.y,lightPos.z);
        myShader.setVec3("ambientLight",1.0f,0.5f,0.5f);
        myShader.setVec3("viewPos",camera.Position.x,camera.Position.y,camera.Position.z);
        
        glBindVertexArray(VAO);

        myShader.setMat4("view",camera.GetViewMatrix());

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Fov),4.0f/3.0f,0.1f,100.0f);
        myShader.setMat4("projection",projection);

        //---画三角形--- 画10个
        for(unsigned int i = 0; i <1 ; i++){
            //使用model之前更新
            glm::mat4 model = glm::mat4(1.0f); //model得在这里初始化，每次都得初始化，不然会叠加
            model = glm::translate(model,cubePositions[i]);
            model = glm::rotate(model,glm::radians(20.0f*i),glm::vec3(1.0f,0.0f,0.0f));
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
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


