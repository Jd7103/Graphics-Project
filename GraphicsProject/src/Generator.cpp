#include "Generator.h"

Generator::Generator(Shader& shader, Shader& spawnShader, const std::vector<std::string>& buildingPaths)
    : shader(shader), spawnShader(spawnShader) {

    //Allocate Space for Instance Buffers and Model Matrices Vectors According to Number of Building Models
    buildingModels.reserve(buildingPaths.size());
    instanceVBOs.resize(buildingPaths.size());
    modelMatrices.resize(buildingPaths.size());

    //Load Building Models
    for (const auto& path : buildingPaths) {
        buildingModels.push_back(std::make_shared<Model>(path));
    }

    //Load Flat Terrain Geometry
    terrainTemplate = std::make_unique<Terrain>(shader);

    //Reserve Space For Model Matrices for Each Building Model
    for (auto& matrices : modelMatrices) {
        matrices.reserve(BUILDINGS_PER_CHUNK * (VIEW_DISTANCE * 2 + 1) * (VIEW_DISTANCE * 2 + 1));
    }

    //Reserve Space for All Visible Terrain Tiles
    terrainMatrices.reserve((VIEW_DISTANCE * 2 + 1) * (VIEW_DISTANCE * 2 + 1));

    setupInstanceBuffers();

    //Load Spire Model for Spawn/Origing Chunk
    std::string spirePath = std::string(PROJECT_ROOT) + "/assets/models/spire/spire.gltf";
    spireModel = std::make_shared<Model>(spirePath);

    spireMatrix = glm::mat4(1.0f);
    spireMatrix = glm::scale(spireMatrix, glm::vec3(20.0f));
    spireMatrix = glm::translate(spireMatrix, glm::vec3(0.0f, 50.0f, 0.0f));
}

void Generator::setupInstanceBuffers() {

    //Setup Instance Buffers for Each Building Model 
    for (size_t i = 0; i < buildingModels.size(); i++) {
        glGenBuffers(1, &instanceVBOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[i]);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(glm::mat4) * modelMatrices[i].capacity(),
            nullptr, GL_DYNAMIC_DRAW);

        //Setup VAO for Instancing for Each Mesh in Each Model
        for (auto& mesh : buildingModels[i]->meshes) {
            glBindVertexArray(mesh.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[i]);

            for (int j = 0; j < 4; j++) {
                glEnableVertexAttribArray(7 + j);
                glVertexAttribPointer(7 + j, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * j));
                glVertexAttribDivisor(7 + j, 1);
            }
        }
    }
    //Buildings

    //Terrain
    glGenBuffers(1, &terrainInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * terrainMatrices.capacity(), nullptr, GL_DYNAMIC_DRAW);

    terrainTemplate->setupInstancedRendering(terrainMatrices.capacity());

}

void Generator::generateChunk(const glm::ivec2& position) {

    ChunkData chunk;
    chunk.position = position;
    chunk.seed = generateChunkSeed(position);

    //Set Aside Spawn/Origin Chunk for Perlin Noise Park
    if (chunk.position == glm::ivec2(0, 0)) {
        terrainTemplate->generateHeightmapMesh();
    }
    else {

        //Calculate Building Positions on a 3x3 Grid Within the Chunk
        const float halfChunkSize = CHUNK_SIZE / 2.0f;
        const float buildingOffset = (halfChunkSize - (ROAD_WIDTH / 2.0f)) * 0.75;

        for (int x = -1; x <= 1; x++) {
            for (int z = -1; z <= 1; z++) {
                BuildingData building;
                float baseX = x == 0 ? 0 : (x > 0 ? buildingOffset : -buildingOffset);
                float baseZ = z == 0 ? 0 : (z > 0 ? buildingOffset : -buildingOffset);
                building.position = glm::vec3(baseX, -50.0f, baseZ);

                uint32_t buildingSeed = generateChunkSeed(position + glm::ivec2(x, z));

                building.modelIndex = selectBuildingWeighted(buildingSeed, buildingModels.size());

                chunk.buildings.push_back(building);
            }
        }
    }

    chunks[position] = chunk;

}

size_t Generator::selectBuildingWeighted(uint32_t seed, size_t numBuildingTypes) {

    std::mt19937 rng(seed); //Pseudo Random Number Generator w/ Mersenne Twisters, based on Chunk Seed

    std::vector<int> weights = { 20, 25, 20, 15, 10, 10};

    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    return static_cast<size_t>(dist(rng));

}

uint32_t Generator::generateChunkSeed(const glm::ivec2& position) const {

    //Seed Generation is Deterministic, Based on World Space Coords, so that Chunks can be Reconstructed Faithfully
    return static_cast<uint32_t>(
        static_cast<uint32_t>(position.x) * static_cast<uint32_t>(12345) +
        static_cast<uint32_t>(position.y) * static_cast<uint32_t>(67890)
        );

}

glm::ivec2 Generator::worldToChunkCoords(const glm::vec3& worldPos) const {

    return glm::ivec2(
        static_cast<int>(std::floor(worldPos.x / CHUNK_SIZE)),
        static_cast<int>(std::floor(worldPos.z / CHUNK_SIZE))
    );

}

std::vector<glm::ivec2> Generator::getVisibleChunks(const glm::ivec2& centerChunk, const glm::vec3& viewDir) const {

    std::vector<glm::ivec2> visibleChunks;
    visibleChunks.reserve((VIEW_DISTANCE * 2 + 1) * (VIEW_DISTANCE * 2 + 1));

    for (int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; x++) {
        for (int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; z++) {
            visibleChunks.push_back(centerChunk + glm::ivec2(x, z));
        }
    }

    return visibleChunks;

}

void Generator::update(const Camera& camera) {
    glm::ivec2 currentChunk = worldToChunkCoords(camera.Position);
    std::vector<glm::ivec2> visibleChunks = getVisibleChunks(currentChunk, camera.Front);

    //Generate Chunks
    for (const auto& chunkPos : visibleChunks) {
        if (chunks.find(chunkPos) == chunks.end()) {
            generateChunk(chunkPos);
        }
    }

    //Cull Far Away Chunks
    for (auto i = chunks.begin(); i != chunks.end();) {
        if (std::find(visibleChunks.begin(), visibleChunks.end(), i->first) == visibleChunks.end()) {
            i = chunks.erase(i);
        }
        else {
            ++i;
        }
    }

    //Clear Model Matrices
    for (auto& matrices : modelMatrices) {
        matrices.clear();
    }

    terrainMatrices.clear();

    //Update Visible Chunks
    for (const auto& pair : chunks) {
        const glm::ivec2& pos = pair.first;
        const ChunkData& chunk = pair.second;

        glm::mat4 chunkMatrix = glm::translate(glm::mat4(1.0f),
            glm::vec3(pos.x * CHUNK_SIZE, 0.0f, pos.y * CHUNK_SIZE));

        //Set Aside Spawn/Origin Chunk
        if (pos != glm::ivec2(0, 0)) {
            terrainMatrices.push_back(chunkMatrix);
        }

        //Sort Buildings by Model and Create Transformations
        for (const auto& building : chunk.buildings) {
            glm::mat4 model = chunkMatrix;
            model = glm::translate(model, building.position);
            model = glm::scale(model, glm::vec3(BUILDING_SCALE));
            modelMatrices[building.modelIndex].push_back(model);
        }
    }
}


void Generator::render(Shader& shader, Shader& spawnShader, Shader& roadShader) {

    //Flat Terrain (Instanced)
    roadShader.use();
    glBindBuffer(GL_ARRAY_BUFFER, terrainInstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        terrainMatrices.size() * sizeof(glm::mat4),
        terrainMatrices.data());

    if (!terrainMatrices.empty()) {
        terrainTemplate->renderInstanced(roadShader, terrainMatrices);
    }

    //Perlin Noise Spawn/Origin Chunk (Not Instanced)
    auto originChunk = chunks.find(glm::ivec2(0, 0));
    if (originChunk != chunks.end()) {
        spawnShader.use();
        terrainTemplate->renderHeightmap(spawnShader);
        spawnShader.setMat4("model", spireMatrix);
        spireModel->render(spawnShader);
    }

    //Buildings (Instanced)
    shader.use();
    for (size_t i = 0; i < buildingModels.size(); i++) {
        if (modelMatrices[i].empty()) continue;

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[i]);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            modelMatrices[i].size() * sizeof(glm::mat4),
            modelMatrices[i].data());

        buildingModels[i]->render(shader, true, modelMatrices[i].size());
    }
}

Generator::~Generator() {
    for (GLuint vbo : instanceVBOs) {
        if (glIsBuffer(vbo)) {
            glDeleteBuffers(1, &vbo);
        }
    }
    glDeleteBuffers(1, &terrainInstanceVBO);
}