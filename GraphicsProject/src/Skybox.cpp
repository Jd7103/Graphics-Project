#include "Skybox.h"
using namespace std;

const string cubeAssets = string(PROJECT_ROOT) + "/assets/cubemaps/sky/";
const string cubeFaces[6] = {cubeAssets + "pos_x.jpg", cubeAssets + "neg_x.jpg", cubeAssets + "pos_y.jpg", 
							 cubeAssets + "neg_y.jpg", cubeAssets + "pos_z.jpg", cubeAssets + "neg_z.jpg", };

GLenum cube[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,
				   GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
				   GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
				   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
				   GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
				   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

float skyboxVertices[] = {
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] = {
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

GLuint skyboxVAO, skyboxVBO, skyboxEBO, skyboxShader, cubemapTexture;

Skybox::Skybox() {}

void Skybox::init() {

	string vert = string(PROJECT_ROOT) + "/src/shaders/skybox.vert";
	string frag = string(PROJECT_ROOT) + "/src/shaders/skybox.frag";
	skyboxShader = LoadShadersFromFile(vert.c_str(), frag.c_str());

	glUseProgram(skyboxShader); 
	glUniform1i(glGetUniformLocation(skyboxShader, "textureSampler"), 0);

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int i = 0; i < 6; i++) {
		int width, height, channels;
		unsigned char* img = stbi_load(cubeFaces[i].c_str(), &width, &height, &channels, 0);

		if (img) {
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D(cube[i], 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		}
		else {
			cout << "Failed to load texture " << cubeFaces[i] << std::endl;
		}
		stbi_image_free(img);
	}
}

void Skybox::render(Camera camera, unsigned int width, unsigned int height, glm::mat4 view, glm::mat4 projection) {
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);

	glUseProgram(skyboxShader);

	view = glm::mat4(glm::mat3(view));

	glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
	glCullFace(GL_BACK);
}

void Skybox::deleteBuffers() {

}