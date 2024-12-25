#ifndef TERRAIN_CLASS_H
#define TERRAIN_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stbi/stb_image.h>
#include <string>
#include <iostream>
#include <vector>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

class Terrain {
public:
    Terrain(Shader& shader);
    void render(Shader& shader);
    void renderInstanced(Shader& shader, const std::vector<glm::mat4>& modelMatrices);
    void setupInstancedRendering(size_t maxInstances);
    void deleteBuffers();

private:
    float originVertices[12] = {
        -500.0f, -50.0f,  500.0f,
         500.0f, -50.0f,  500.0f,
         500.0f, -50.0f, -500.0f,
        -500.0f, -50.0f, -500.0f
    };

    GLuint originIndices[6] = {
        0, 2, 3,
        2, 0, 1
    };

    float originNormals[12]{
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    float tileFactor = 100.0f;

    float originUVs[8] = {
        0.0f, (500.0f / 1000.0f) * tileFactor,
        (259.0f / 1000.0f) * tileFactor, (500.0f / 1000.0f) * tileFactor,
        (259.0f / 1000.0f) * tileFactor, 0.0f,
        0.0f, 0.0f
    };

    GLuint terrainVAO, terrainVBO, terrainEBO, textureID;
    GLuint terrainNormal, terrainUV, instanceVBO;
};
#endif