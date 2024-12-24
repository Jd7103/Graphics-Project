#include "Camera.h"

Camera::Camera(glm::vec3 position, float near, float far, unsigned int height, unsigned int width, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {

    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    zNear = near;
    zFar = far;
    screenHeight = height;
    screenWidth = width;
    updateCameraVectors();

}

Camera::Camera(float posX, float posY, float posZ, float near, float far, unsigned int height, unsigned int width, float upX, float upY, float upZ, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {

    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    zNear = near;
    zFar = far;
    screenHeight = height;
    screenWidth = width;
    updateCameraVectors();

}

glm::mat4 Camera::projectionMatrix() {

    return glm::perspective(glm::radians(Zoom), (float)screenWidth / (float)screenHeight, zNear, zFar);

}

glm::mat4 Camera::viewMatrix() {

    return glm::lookAt(Position, Position + Front, Up);

}

void Camera::processKeyboard(Camera_Movement direction) {

    if (direction == FORWARD)
        Position += Front * MovementSpeed;
    if (direction == BACKWARD)
        Position -= Front * MovementSpeed;
    if (direction == LEFT)
        Position -= Right * MovementSpeed;
    if (direction == RIGHT)
        Position += Right * MovementSpeed;
    if (direction == UP)
        Position += Up * MovementSpeed;
    if (direction == DOWN)
        Position -= Up * MovementSpeed;
    if (direction == FAST)
        MovementSpeed = 0.8f;
    if (direction == SLOW)
        MovementSpeed = 0.3f;

}

void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {

    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {

        if (Pitch > 89.0f) 
            Pitch = 89.0f;
        if (Pitch < -89.0f) 
            Pitch = -89.0f;

    }

    updateCameraVectors();

}

void Camera::processMouseScroll(float yoffset) {

    Zoom -= (float)yoffset;
    if (Zoom < 1.0f) 
        Zoom = 1.0f;
    if (Zoom > 90.0f) 
        Zoom = 90.0f;

}

void Camera::updateCameraVectors() {

    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));

}