#ifndef TERRAIN_H
#define TERRAIN_H

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
    void renderInstanced(Shader& shader, const std::vector<glm::mat4>& modelMatrices);
    void renderHeightmap(Shader& shader);
    void setupInstancedRendering(size_t maxInstances);
    void generateHeightmapMesh(int resolution = 100);
    void deleteBuffers();


private:


    //Flat Plane Vertices at the Origin
    float originVertices[12] = {

        -500.0f, -50.0f,  500.0f,
         500.0f, -50.0f,  500.0f,
         500.0f, -50.0f, -500.0f,
        -500.0f, -50.0f, -500.0f

    };

    //Flat Plane Indices
    GLuint originIndices[6] = {

        0, 2, 3,
        2, 0, 1

    };

    //Flat Plane Normals (Straight Up)
    float originNormals[12]{

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f

    };

    //UV Coordinates for a Repeating Grass Texture
    float tileFactor = 100.0f;

    float originUVs[8] = {

        0.0f, (500.0f / 1000.0f) * tileFactor,
        (961.0f / 1000.0f) * tileFactor, (500.0f / 1000.0f) * tileFactor,
        (961.0f / 1000.0f) * tileFactor, 0.0f,
        0.0f, 0.0f

    };

    GLuint terrainVAO, terrainVBO, terrainEBO, grassID, roadID, pathID;
    GLuint terrainNormal, terrainUV, instanceVBO;


    //Perlin Noise

    GLuint heightmapVAO, heightmapVBO, heightmapEBO;
    GLuint heightmapNormal, heightmapUV;
    unsigned int heightmapIndexCount;

    float fade(float t);
    float lerp(float a, float b, float t);
    float grad(int hash, float x, float y);
    float noise(float x, float y);
    float octaveNoise(float x, float y, int octaves, float persistence);
    std::vector<float> generateHeightMap(int resolution);

};
#endif