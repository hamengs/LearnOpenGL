#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up , float fov, float pitch, float yaw)
{
    Position = position;
    Front = front;
    Up = up;
    Fov = fov;
    Pitch = pitch;
    Yaw = yaw;
    Right = glm::normalize(glm::cross(Front,glm::vec3(0.0f,1.0f,0.0f)));
    
}

glm::mat4 Camera::GetViewMatrix(){
    return glm::lookAt(Position,Position+Front,Up);
}

void Camera::ProcessKeyboard(float deltaTime, Camera_Movement direction){
    if(direction == Camera_Movement::FORWARD){
        Position += Front * deltaTime;
    }
    if(direction == Camera_Movement::BACKWARD){
        Position -= Front * deltaTime;
    }
    if(direction == Camera_Movement::LEFT){
        Position -= Right * deltaTime;
    }
    if(direction == Camera_Movement::RIGHT){
        Position += Right * deltaTime;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset){
    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if(Pitch > 89.0f){
        Pitch = 89.0f;
    }
    if(Pitch < -89.0f){
        Pitch = -89.0f;
    }

    glm::vec3 direction;
    direction.y = sin(glm::radians(Pitch));
    direction.x = cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
    direction.z = cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));
    Front = glm::normalize(direction);
    Right = glm::normalize(glm::cross(Front,glm::vec3(0.0f,1.0f,0.0f)));
}

void Camera::ProcessMouseScroll(float yoffset){
    if(Fov >= 1.0f && Fov <= 45.0f){
        Fov -= yoffset;
    }
    if(Fov <= 1.0f){
        Fov = 1.0f;
    }
    if(Fov >= 45.0f){
        Fov = 45.0f;
    }
}