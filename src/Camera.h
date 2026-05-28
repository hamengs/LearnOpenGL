#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {

    public:
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        float Fov;
        float Pitch;
        float Yaw;

        Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up, float fov, float pitch, float yaw);
        glm::mat4 GetViewMatrix();

        void ProcessKeyboard(float deltaTime, Camera_Movement direction);
        void ProcessMouseMovement(float xoffset, float yoffset);
        void ProcessMouseScroll(float yoffset);
};

#endif