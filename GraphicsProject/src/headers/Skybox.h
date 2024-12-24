#ifndef SKYBOX_CLASS_H
#define SKYBOX_CLASS_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include <stbi/stb_image.h>

#include <string>
#include <iostream>

#include "Shader.h"
#include "Camera.h"

class Skybox {

public:

	Skybox(Shader& shader);
	void render(Shader& shader, Camera& camera);
	void deleteBuffers();

private:

    const std::string cubeAssets = std::string(PROJECT_ROOT) + "/assets/cubemaps/sky/";
    std::string cubeFaces[6] = { cubeAssets + "pos_x.jpg", cubeAssets + "neg_x.jpg", cubeAssets + "pos_y.jpg",
                                cubeAssets + "neg_y.jpg", cubeAssets + "pos_z.jpg", cubeAssets + "neg_z.jpg", };

    GLenum cube[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                       GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                       GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                       GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                       GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                       GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

    float skyboxVertices[24] = {
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    };

    unsigned int skyboxIndices[36] = {
        // Right
        1, 6, 2,
        6, 1, 5,
        // Left
        0, 7, 4,
        7, 0, 3,
        // Top
        4, 6, 5,
        6, 4, 7,
        // Bottom
        0, 2, 3,
        2, 0, 1,
        // Back
        0, 5, 1,
        5, 0, 4,
        // Front
        3, 6, 7,
        6, 3, 2
    };

	GLuint skyboxVAO, skyboxVBO, skyboxEBO, textureID;

};
#endif