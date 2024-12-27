#include "Spawn.h"
#include <glm/gtc/constants.hpp>

Spawn::Spawn(Shader& shader, const std::string& spirePath) {
    // Load spire model
    spireModel = std::make_shared<Model>(spirePath);

    // Setup initial spire matrix
    spireMatrix = glm::mat4(1.0f);
    spireMatrix = glm::translate(spireMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    spireMatrix = glm::scale(spireMatrix, glm::vec3(10.0f));

    // Setup the decorative elements
    generateGardenBeds();
    setupWallBuffers();
    setupGardenBuffers();
}

void Spawn::generateGardenBeds() {
    float gapAngleSize = GAP_SIZE / GARDEN_RADIUS;
    float segmentArcLength = (glm::pi<float>() / 2.0f) - gapAngleSize;

    for (int i = 0; i < 4; ++i) {
        float baseAngle = i * glm::pi<float>() / 2.0f + gapAngleSize / 2.0f;
        GardenBed bed;
        bed.position = glm::vec3(
            GARDEN_RADIUS * cos(baseAngle + segmentArcLength / 2.0f),
            0.0f,
            GARDEN_RADIUS * sin(baseAngle + segmentArcLength / 2.0f)
        );
        bed.rotation = baseAngle;
        bed.arcLength = segmentArcLength;
        gardenBeds.push_back(bed);
    }
}

void Spawn::setupWallBuffers() {
    const int NUM_SEGMENTS = 64;  // More segments for smoother circle
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Reserve space for vertices
    vertices.reserve((NUM_SEGMENTS + 1) * 4 * 6);  // 4 vertices per segment, 6 components per vertex

    // Generate vertices
    for (int i = 0; i <= NUM_SEGMENTS; ++i) {
        float angle = (float)i / NUM_SEGMENTS * 2.0f * glm::pi<float>();
        float cosA = cos(angle);
        float sinA = sin(angle);

        // Inner radius vertex
        glm::vec3 innerPos = glm::vec3(
            (WALL_RADIUS - WALL_THICKNESS / 2) * cosA,
            0.0f,
            (WALL_RADIUS - WALL_THICKNESS / 2) * sinA
        );
        glm::vec3 innerNormal = -glm::normalize(glm::vec3(cosA, 0.0f, sinA));

        // Outer radius vertex
        glm::vec3 outerPos = glm::vec3(
            (WALL_RADIUS + WALL_THICKNESS / 2) * cosA,
            0.0f,
            (WALL_RADIUS + WALL_THICKNESS / 2) * sinA
        );
        glm::vec3 outerNormal = glm::normalize(glm::vec3(cosA, 0.0f, sinA));

        // Add vertices for all four corners with their normals
        // Inner bottom
        vertices.push_back(innerPos.x);
        vertices.push_back(innerPos.y);
        vertices.push_back(innerPos.z);
        vertices.push_back(innerNormal.x);
        vertices.push_back(innerNormal.y);
        vertices.push_back(innerNormal.z);

        // Outer bottom
        vertices.push_back(outerPos.x);
        vertices.push_back(outerPos.y);
        vertices.push_back(outerPos.z);
        vertices.push_back(outerNormal.x);
        vertices.push_back(outerNormal.y);
        vertices.push_back(outerNormal.z);

        // Inner top
        vertices.push_back(innerPos.x);
        vertices.push_back(innerPos.y + WALL_HEIGHT);
        vertices.push_back(innerPos.z);
        vertices.push_back(innerNormal.x);
        vertices.push_back(innerNormal.y);
        vertices.push_back(innerNormal.z);

        // Outer top
        vertices.push_back(outerPos.x);
        vertices.push_back(outerPos.y + WALL_HEIGHT);
        vertices.push_back(outerPos.z);
        vertices.push_back(outerNormal.x);
        vertices.push_back(outerNormal.y);
        vertices.push_back(outerNormal.z);
    }

    // Generate indices for the wall
    for (int i = 0; i < NUM_SEGMENTS; ++i) {
        int baseIndex = i * 4;  // 4 vertices per segment

        // Wall faces
        // Bottom face
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 5);

        // Top face
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 7);

        // Inner face
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 6);

        // Outer face
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 7);
    }

    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);

    glBindVertexArray(wallVAO);

    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    wallIndexCount = indices.size();
}

void Spawn::setupGardenBuffers() {
    const int SEGMENTS = 32;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve((SEGMENTS + 1) * 12); // 2 vertices per point, 6 components per vertex

    for (int i = 0; i <= SEGMENTS; ++i) {
        float t = (float)i / SEGMENTS * glm::pi<float>() / 2.0f;  // Quarter circle for each bed
        float cosT = cos(t);
        float sinT = sin(t);

        // Inner radius vertices with normals
        vertices.push_back((GARDEN_RADIUS - GARDEN_WIDTH / 2) * cosT);
        vertices.push_back(0.0f);
        vertices.push_back((GARDEN_RADIUS - GARDEN_WIDTH / 2) * sinT);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);

        // Outer radius vertices with normals
        vertices.push_back((GARDEN_RADIUS + GARDEN_WIDTH / 2) * cosT);
        vertices.push_back(0.0f);
        vertices.push_back((GARDEN_RADIUS + GARDEN_WIDTH / 2) * sinT);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
    }

    for (int i = 0; i < SEGMENTS; ++i) {
        indices.push_back(2 * i);
        indices.push_back(2 * i + 1);
        indices.push_back(2 * i + 2);

        indices.push_back(2 * i + 1);
        indices.push_back(2 * i + 3);
        indices.push_back(2 * i + 2);
    }

    glGenVertexArrays(1, &gardenVAO);
    glGenBuffers(1, &gardenVBO);
    glGenBuffers(1, &gardenEBO);

    glBindVertexArray(gardenVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gardenVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gardenEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Spawn::render(Shader& shader) {
    shader.use();

    // Render spire
    shader.setInt("useTexture", 1);  // Using model's textures
    shader.setMat4("model", spireMatrix);
    spireModel->render(shader);  // No instancing

    shader.setInt("useTexture", 0);  // Switch back to solid colors for other elements

    // Render wall (no instancing needed)
    glm::mat4 wallMatrix = glm::mat4(1.0f);
    shader.setMat4("model", wallMatrix);
    shader.setVec3("diffuseColour", glm::vec3(0.8f, 0.8f, 0.8f));
    glBindVertexArray(wallVAO);
    glDrawElements(GL_TRIANGLES, wallIndexCount, GL_UNSIGNED_INT, 0);

    // Render garden beds
    shader.setVec3("diffuseColour", glm::vec3(0.45f, 0.32f, 0.22f));
    for (const auto& bed : gardenBeds) {
        glm::mat4 gardenMatrix = glm::mat4(1.0f);
        gardenMatrix = glm::rotate(gardenMatrix, bed.rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", gardenMatrix);

        glBindVertexArray(gardenVAO);
        glDrawElements(GL_TRIANGLES, 192, GL_UNSIGNED_INT, 0);
    }
}

Spawn::~Spawn() {
    glDeleteVertexArrays(1, &wallVAO);
    glDeleteBuffers(1, &wallVBO);
    glDeleteBuffers(1, &wallEBO);

    glDeleteVertexArrays(1, &gardenVAO);
    glDeleteBuffers(1, &gardenVBO);
    glDeleteBuffers(1, &gardenEBO);
}