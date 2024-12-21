#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stbi/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "shader.h"
#include "Camera.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

class Model
{
public:
    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    Model(string const& path, bool gamma = false) : gammaCorrection(gamma) {
        loadModel(path);
    }

    void render(GLuint& shader, Camera camera, int height, int width, glm::mat4 view, glm::mat4 projection, glm::vec3 lightDir,
        glm::vec3 scale, glm::vec3 translate, float rotateAngle, glm::vec3 rotateAxis);

private:

    void loadModel(string const& path);

    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

    GLuint TextureFromFile(const char* path, const string& directory, bool gamma = false);
};
#endif