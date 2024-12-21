#include "Terrain.h"
using namespace std;

Terrain::Terrain() {}

float terrainVertices[] = {
	-500.0f, -50.0f,  500.0f,
	 500.0f, -50.0f,  500.0f,
	 500.0f, -50.0f, -500.0f,
	-500.0f, -50.0f, -500.0f
};

float tileFactor = 100.0f;

float terrainUVs[] = {
    0.0f, (500.0f / 1000.0f)* tileFactor,
    (259.0f / 1000.0f)* tileFactor, (500.0f / 1000.0f) * tileFactor,
    (259.0f / 1000.0f)* tileFactor, 0.0f,
    0.0f, 0.0f
};

unsigned int terrainIndices[] = {
	0, 3, 2,
	2, 1, 0
};

float terrainNormals[]{
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f
};

GLuint terrainVAO, terrainVBO, terrainEBO, terrainTexture, terrainUV, terrainNormal;

void Terrain::initPlanar(GLuint& shader) {

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "textureSampler"), 0);

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);
    glGenBuffers(1, &terrainUV);
    glGenBuffers(1, &terrainNormal);

    glBindVertexArray(terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), &terrainVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainIndices), &terrainIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, terrainNormal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(terrainNormals), terrainNormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, terrainUV);
    glBufferData(GL_ARRAY_BUFFER, sizeof(terrainUVs), terrainUVs, GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenTextures(1, &terrainTexture);
    glBindTexture(GL_TEXTURE_2D, terrainTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, channels;
    string texturePath = string(PROJECT_ROOT) + "/assets/textures/asphalt.jpg";
    unsigned char* img = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

    if (img) {
        stbi_set_flip_vertically_on_load(true);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        cout << "Failed to load texture: " << texturePath << endl;
    }

    stbi_image_free(img);
	
}

void Terrain::renderPlanar(GLuint& shader, Camera camera, unsigned int width, unsigned int height,
    glm::mat4 view, glm::mat4 projection, glm::vec3 lightDir) {
    glUseProgram(shader);
    glCullFace(GL_FRONT);

    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(camera.position));
    glUniform3fv(glGetUniformLocation(shader, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform1i(glGetUniformLocation(shader, "useTexture"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainTexture);

    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glCullFace(GL_BACK);
}
