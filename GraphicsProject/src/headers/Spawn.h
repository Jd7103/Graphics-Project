#ifndef SPAWN_H
#define SPAWN_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include "Shader.h"
#include "Model.h"

class Spawn {
public:
    Spawn(Shader& shader, const std::string& spirePath);
    void render(Shader& shader);
    ~Spawn();

private:
    struct GardenBed {
        glm::vec3 position;
        float rotation;
        float arcLength;
    };

    // Constants
    static constexpr float WALL_RADIUS = 20.0f;
    static constexpr float GARDEN_RADIUS = 35.0f;
    static constexpr float WALL_HEIGHT = 2.0f;
    static constexpr float WALL_THICKNESS = 0.5f;
    static constexpr float GARDEN_WIDTH = 3.0f;
    static constexpr float GAP_SIZE = 50.0f;

    std::shared_ptr<Model> spireModel;
    std::vector<GardenBed> gardenBeds;

    // Transform matrices
    glm::mat4 spireMatrix;

    // Geometry buffers
    GLuint wallVAO, wallVBO, wallEBO;
    GLuint gardenVAO, gardenVBO, gardenEBO;
    unsigned int wallIndexCount;

    void setupWallBuffers();
    void setupGardenBuffers();
    void generateGardenBeds();
};

#endif