#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Movement {

    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    FAST,
    SLOW

};


const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 0.1f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 90.0f;


class Camera {

public:
    
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
   
    float Yaw;
    float Pitch;
    
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    float zNear;
    float zFar;
    unsigned int screenHeight;
    unsigned int screenWidth;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float near = 0.5f, float far = 2500.0f, unsigned int height = 1080, unsigned int width = 1920, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    Camera(float posX, float posY, float posZ, float near, float far, unsigned int height, unsigned int width, float upX, float upY, float upZ, float yaw, float pitch);

    glm::mat4 viewMatrix();

    glm::mat4 Camera::projectionMatrix();

    void processKeyboard(Camera_Movement direction);

    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    void processMouseScroll(float yoffset);

private:
    
    void updateCameraVectors();

};
#endif