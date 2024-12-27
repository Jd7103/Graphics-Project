#include "Terrain.h"

Terrain::Terrain(Shader& shader) {

    shader.use();
    shader.setInt("textureSampler", 0);

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);
    glGenBuffers(1, &terrainUV);
    glGenBuffers(1, &terrainNormal);

    glBindVertexArray(terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(originVertices), &originVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(originIndices), &originIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, terrainNormal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(originNormals), originNormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, terrainUV);
    glBufferData(GL_ARRAY_BUFFER, sizeof(originUVs), originUVs, GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    string textureDirectory = string(PROJECT_ROOT) + "/assets/textures/";
    char* texturePath = "grass.jpg";
    textureID = TextureFromFile(texturePath, textureDirectory);

    heightmapVAO = 0;
    heightmapVBO = 0;
    heightmapEBO = 0;
    heightmapNormal = 0;
    heightmapUV = 0;
    heightmapIndexCount = 0;

}

void Terrain::setupInstancedRendering(size_t maxInstances) {

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    glBindVertexArray(terrainVAO);

    std::size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(7 + i); 
        glVertexAttribPointer(7 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
            (void*)(i * vec4Size));
        glVertexAttribDivisor(7 + i, 1);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Terrain::renderInstanced(Shader& shader, const std::vector<glm::mat4>& modelMatrices) {

    if (modelMatrices.empty()) return;

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBindVertexArray(terrainVAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, modelMatrices.size());

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void Terrain::deleteBuffers() {

    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteBuffers(1, &terrainEBO);
    glDeleteBuffers(1, &instanceVBO);

    glDeleteVertexArrays(1, &heightmapVAO);
    glDeleteBuffers(1, &heightmapVBO);
    glDeleteBuffers(1, &heightmapEBO);
    glDeleteBuffers(1, &heightmapNormal);
    glDeleteBuffers(1, &heightmapUV);

}


//Perlin
void Terrain::generateHeightmapMesh(int resolution) {

    std::vector<float> heightMap = generateHeightMap(resolution);
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<unsigned int> indices;

    float gridSize = 1000.0f / (resolution - 1);

    //Vertices
    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {

            bool isEdge = (x == 0 || x == resolution - 1 || z == 0 || z == resolution - 1);

            float xPos = x * gridSize - 500.0f;

            //Clamp Edges to Flat Plane Height
            float yPos = isEdge ? -50.0f : heightMap[z * resolution + x] - 50.0f;

            float zPos = z * gridSize - 500.0f;

            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);

            //Normals
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            if (x > 0 && x < resolution - 1 && z > 0 && z < resolution - 1) {
                float hL = heightMap[z * resolution + (x - 1)];
                float hR = heightMap[z * resolution + (x + 1)];
                float hD = heightMap[(z - 1) * resolution + x];
                float hU = heightMap[(z + 1) * resolution + x];

                float scale = 0.5f * gridSize;
                glm::vec3 tangent(2.0f * scale, hR - hL, 0.0f);
                glm::vec3 bitangent(0.0f, hU - hD, 2.0f * scale);
                normal = glm::normalize(glm::cross(tangent, bitangent));
            }

            normals.push_back(-normal.x);
            normals.push_back(-normal.y);
            normals.push_back(-normal.z);

            //UV Coordinates
            uvs.push_back(float(x) / (resolution - 1) * 10.0f);
            uvs.push_back(float(z) / (resolution - 1) * 10.0f);
        }
    }

    //Indices
    for (int z = 0; z < resolution - 1; z++) {
        for (int x = 0; x < resolution - 1; x++) {
            unsigned int topLeft = z * resolution + x;
            unsigned int bottomLeft = (z + 1) * resolution + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    heightmapIndexCount = indices.size();

    //Buffers
    glGenVertexArrays(1, &heightmapVAO);
    glGenBuffers(1, &heightmapVBO);
    glGenBuffers(1, &heightmapEBO);
    glGenBuffers(1, &heightmapNormal);
    glGenBuffers(1, &heightmapUV);

    glBindVertexArray(heightmapVAO);

    glBindBuffer(GL_ARRAY_BUFFER, heightmapVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, heightmapEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, heightmapNormal);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, heightmapUV);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, heightmapVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, heightmapNormal);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, heightmapUV);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

}

std::vector<float> Terrain::generateHeightMap(int resolution) {

    std::vector<float> heightMap(resolution * resolution);

    float offsetX = static_cast<float>(rand()) / RAND_MAX * 1000.0f;
    float offsetY = static_cast<float>(rand()) / RAND_MAX * 1000.0f;

    for (int y = 0; y < resolution; y++) {
        for (int x = 0; x < resolution; x++) {
            float sampleX = (x + offsetX) * 0.03f;
            float sampleY = (y + offsetY) * 0.03f;

            float value = octaveNoise(sampleX, sampleY, 6, 0.5f);
            value = value * value;
            heightMap[y * resolution + x] = value * 100.0f;
        }
    }

    return heightMap;
}

void Terrain::renderHeightmap(Shader& shader) {
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBindVertexArray(heightmapVAO);
    glDrawElements(GL_TRIANGLES, heightmapIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//Noise Functions
float Terrain::fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Terrain::lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float Terrain::grad(int hash, float x, float y) {

    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);

}

float Terrain::noise(float x, float y) {

    // Permutation Table from Ken Perlin's Original Implementation
    static const int p[512] = {
                      151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225,
                      140,  36, 103,  30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148,
                      247, 120, 234,  75,   0,  26, 197,  62,  94, 252, 219, 203, 117,  35,  11,  32,
                       57, 177,  33,  88, 237, 149,  56,  87, 174,  20, 125, 136, 171, 168,  68, 175,
                       74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231,  83, 111, 229, 122,
                       60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143,  54,
                       65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169,
                      200, 196, 135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,
                       52, 217, 226, 250, 124, 123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212,
                      207, 206,  59, 227,  47,  16,  58,  17, 182, 189,  28,  42, 223, 183, 170, 213,
                      119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172,   9,
                      129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 112, 104,
                      218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,
                       81,  51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157,
                      184,  84, 204, 176, 115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93,
                      222, 114,  67,  29,  24,  72, 243, 141, 128, 195,  78,  66, 215,  61, 156, 180
    };

    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;

    x -= floor(x);
    y -= floor(y);

    float u = fade(x);
    float v = fade(y);

    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];

    return lerp(
        lerp(grad(p[AA], x, y), grad(p[BA], x - 1, y), u),
        lerp(grad(p[AB], x, y - 1), grad(p[BB], x - 1, y - 1), u),
        v);

}

float Terrain::octaveNoise(float x, float y, int octaves, float persistence) {

    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += noise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue;

}