#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;

    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    GLuint id;
    string type;
    string path;
};

class Mesh {
public:
    vector<Vertex>       vertices;
    vector<GLuint>       indices;
    vector<Texture>      textures;
    GLuint VAO;

    bool useTexture;
    glm::vec3 diffuseColor;

    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, glm::vec3 diffuseColor);

    void render(GLuint& shader);

private:
    GLuint VBO, EBO;

    void setupMesh();
};
#endif