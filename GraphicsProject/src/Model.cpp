#include "Model.h"

//Attrib: Adapted from LearnOpenGL Model Loading Template, with some Alterations to Work for Embedded Textures and Instancing
Model::Model(string const& path, bool gamma) : gammaCorrection(gamma) {

    loadModel(path);

}

void Model::render(Shader& shader, bool instanced, size_t instanceCount) {
    
    for (GLuint i = 0; i < meshes.size(); i++) {
        
        shader.setVec3("diffuseColour", meshes[i].diffuseColor);

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;

        for (unsigned int j = 0; j < meshes[i].textures.size(); j++) {
            glActiveTexture(GL_TEXTURE0 + j);
            string number;
            string name = meshes[i].textures[j].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);

            shader.setInt((name + number).c_str(), j);
            glBindTexture(GL_TEXTURE_2D, meshes[i].textures[j].id);
        }

        shader.setInt("useTexture", meshes[i].textures.empty() ? 0 : 2);

        glBindVertexArray(meshes[i].VAO);
        if (instanced) {
            glDrawElementsInstanced(GL_TRIANGLES, meshes[i].indices.size(), GL_UNSIGNED_INT, 0, instanceCount);
        }
        else {
            glDrawElements(GL_TRIANGLES, meshes[i].indices.size(), GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
    }

    glActiveTexture(GL_TEXTURE0);
    shader.setInt("useTexture", 0);
}

void Model::loadModel(string const& path) {

    Assimp::Importer importer;
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {

        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;

    }

    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);

}

void Model::processNode(aiNode* node, const aiScene* scene) {

    for (GLuint i = 0; i < node->mNumMeshes; i++) {

        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));

    }

    for (GLuint i = 0; i < node->mNumChildren; i++) {

        processNode(node->mChildren[i], scene);

    }

}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {

    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;

    for (GLuint i = 0; i < mesh->mNumVertices; i++) {

        Vertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->HasNormals()) {

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

        }

        if (mesh->mTextureCoords[0]) {

            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    for (GLuint i = 0; i < mesh->mNumFaces; i++) {

        aiFace face = mesh->mFaces[i];

        for (GLuint j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);

    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    aiColor4D diffColor(1.0f, 1.0f, 1.0f, 1.0f); 
    if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffColor)) {
     
        diffuseColor = glm::vec3(diffColor.r, diffColor.g, diffColor.b);
    }
    else {
      
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            diffuseColor = glm::vec3(1.0f); 
        }
        else {
           
            diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
        }
    }

    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    return Mesh(vertices, indices, textures, diffuseColor);

}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {

    vector<Texture> textures;

    for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        cout << i << endl;

        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
        
        if (embeddedTexture) {
                GLuint textureID;
                glGenTextures(1, &textureID);

                GLenum format = embeddedTexture->mHeight > 0 ? GL_RGBA : GL_RGB;

                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, format, embeddedTexture->mWidth, embeddedTexture->mHeight,
                    0, format, GL_UNSIGNED_BYTE, embeddedTexture->pcData);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                Texture texture;
                texture.id = textureID;
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
        }
        else { 

            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();

            cout << texture.path << endl;

            textures.push_back(texture);

        }
    }
    return textures;

}


GLuint TextureFromFile(const char* path, const string& directory, bool gamma) {

    string filename = string(path);
    filename = directory + '/' + filename;

    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data) {

        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    else {

        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);

    }

    return textureID;

}