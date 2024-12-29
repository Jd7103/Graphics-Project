#ifndef GENERATOR_H
#define GENERATOR_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>
#include <random>
#include <string>
#include <vector>
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "Terrain.h"

// Hash function for ivec2 Data Types
struct Vec2Hash {

    size_t operator()(const glm::ivec2& v) const {

        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);

    }
};

class Generator {

public:

    Generator(Shader& shader, Shader& spawnShader, const std::vector<std::string>& buildingPaths);
    void update(const Camera& camera);
    void render(Shader& shader, Shader& spawnShader, Shader& roadShader);
    ~Generator();

private:

    struct BuildingData {
        glm::vec3 position;
        float rotation;
        size_t modelIndex; 
    };

    struct ChunkData {
        glm::ivec2 position;
        uint32_t seed;
        std::vector<BuildingData> buildings;
    };

    // Constants
    static constexpr float CHUNK_SIZE = 1000.0f;
    static constexpr int BUILDINGS_PER_CHUNK = 9;
    static constexpr float BUILDING_SCALE = 100.0f;
    static constexpr float ROAD_WIDTH = 100.0f;
    static constexpr int VIEW_DISTANCE = 4;

    // Shaders
    Shader& shader;
    Shader& spawnShader;

    //Models and Instance Matrics
    std::vector<std::shared_ptr<Model>> buildingModels;
    std::vector<std::vector<glm::mat4>> modelMatrices;
    
    std::shared_ptr<Model> spireModel;
    std::unique_ptr<Terrain> terrainTemplate;
    
    //Matrices
    glm::mat4 spireMatrix;
    std::vector<glm::mat4> terrainMatrices;
    
    //Chunks
    std::unordered_map<glm::ivec2, ChunkData, Vec2Hash> chunks;
    
    //Buffers
    std::vector<GLuint> instanceVBOs;
    GLuint terrainInstanceVBO;

    uint32_t generateChunkSeed(const glm::ivec2& position) const;
    glm::ivec2 worldToChunkCoords(const glm::vec3& worldPos) const;
    std::vector<glm::ivec2> getVisibleChunks(const glm::ivec2& centerChunk, const glm::vec3& viewDir) const;
    void generateChunk(const glm::ivec2& position);
    size_t selectBuildingWeighted(uint32_t seed, size_t numBuildingTypes);
    void setupInstanceBuffers();

};

#endif