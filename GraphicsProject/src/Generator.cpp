#include "Generator.h"

Generator::Generator(Shader& shader, Shader& spawnShader, const std::string& buildingModelPath) : shader(shader), spawnShader(spawnShader) {

    buildingModel = std::make_shared<Model>(buildingModelPath);
    terrainTemplate = std::make_unique<Terrain>(shader);

    buildingMatrices.reserve(BUILDINGS_PER_CHUNK * (VIEW_DISTANCE * 2 + 1) * (VIEW_DISTANCE * 2 + 1));
    terrainMatrices.reserve((VIEW_DISTANCE * 2 + 1) * (VIEW_DISTANCE * 2 + 1));

    setupInstanceBuffers();
}

void Generator::setupInstanceBuffers() {

    // Buildings
    glGenBuffers(1, &buildingInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, buildingInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(glm::mat4) * buildingMatrices.capacity(),
        nullptr, GL_DYNAMIC_DRAW);

    // Setup building VAO for instancing
    for (auto& mesh : buildingModel->meshes) {
        glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, buildingInstanceVBO);

        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(7 + i);
            glVertexAttribPointer(7 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(7 + i, 1);
        }
    }

    // Terrain
    glGenBuffers(1, &terrainInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(glm::mat4) * terrainMatrices.capacity(),
        nullptr, GL_DYNAMIC_DRAW);

    terrainTemplate->setupInstancedRendering(terrainMatrices.capacity());

}

void Generator::generateChunk(const glm::ivec2& position) {

    ChunkData chunk;
    chunk.position = position;
    chunk.seed = generateChunkSeed(position);

    if (chunk.position == glm::ivec2(0, 0)) {
        // Generate heightmap terrain for origin
        terrainTemplate->generateHeightmapMesh();
    } 
    else {

        // Buildings on Edge are Closer to the Edge for Seamless Chunk Transitions
        const float halfChunkSize = CHUNK_SIZE / 2.0f;
        const float buildingOffset = (halfChunkSize - (ROAD_WIDTH / 2.0f)) * 0.75;

        for (int x = -1; x <= 1; x++) {
            for (int z = -1; z <= 1; z++) {
                BuildingData building;

                //Relative Position of Building within Chunk
                float baseX = x == 0 ? 0 : (x > 0 ? buildingOffset : -buildingOffset);
                float baseZ = z == 0 ? 0 : (z > 0 ? buildingOffset : -buildingOffset);

                building.position = glm::vec3(baseX, -50.0f, baseZ);

                chunk.buildings.push_back(building);
            }
        }
    }

    chunks[position] = chunk;

}

uint32_t Generator::generateChunkSeed(const glm::ivec2& position) const {

    // Seed Generation is Deterministic, Based on World Space Coords, so that Chunks can be Reconstructed Faithfully
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

    // Get all chunks in view distance
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

    // Generate Chunks
    for (const auto& chunkPos : visibleChunks) {
        if (chunks.find(chunkPos) == chunks.end()) {
            generateChunk(chunkPos);
        }
    }

    // Cull Far Away Chunks
    for (auto it = chunks.begin(); it != chunks.end();) {
        if (std::find(visibleChunks.begin(), visibleChunks.end(), it->first)
            == visibleChunks.end()) {
            it = chunks.erase(it);
        }
        else {
            ++it;
        }
    }

    buildingMatrices.clear();
    terrainMatrices.clear();

    for (const auto& pair : chunks) {
        const glm::ivec2& pos = pair.first;
        const ChunkData& chunk = pair.second;

        // 2D Chunk Coords (i.e. 0,0 is the origin chunk) to World Space Coords
        glm::mat4 chunkMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x * CHUNK_SIZE, 0.0f, pos.y * CHUNK_SIZE));

        if (pos != glm::ivec2(0, 0)) {
            terrainMatrices.push_back(chunkMatrix);
        }

        // Building Chunk-Relative Coords to World Space Coords
        for (const auto& building : chunk.buildings) {
            
            glm::mat4 model = chunkMatrix;
            model = glm::translate(model, building.position);
            model = glm::scale(model, glm::vec3(BUILDING_SCALE));
            buildingMatrices.push_back(model);

        }
    }
}

void Generator::render(Shader& shader, Shader& spawnShader) {
    
    // Buildings
    glBindBuffer(GL_ARRAY_BUFFER, buildingInstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        buildingMatrices.size() * sizeof(glm::mat4),
        buildingMatrices.data());

    if (!buildingMatrices.empty()) {
        shader.use();
        buildingModel->render(shader, true, buildingMatrices.size());
    }

    //Terrain
    glBindBuffer(GL_ARRAY_BUFFER, terrainInstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        terrainMatrices.size() * sizeof(glm::mat4),
        terrainMatrices.data());

    auto originChunk = chunks.find(glm::ivec2(0, 0));
    if (originChunk != chunks.end()) {
        spawnShader.use();
        terrainTemplate->renderHeightmap(spawnShader);
    }

    if (!terrainMatrices.empty())
        shader.use();
        terrainTemplate->renderInstanced(shader, terrainMatrices);
}

Generator::~Generator() {
    glDeleteBuffers(1, &buildingInstanceVBO);
    glDeleteBuffers(1, &terrainInstanceVBO);
}