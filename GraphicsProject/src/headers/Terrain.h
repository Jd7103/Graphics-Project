#ifndef TERRAIN_CLASS_H
#define TERRAIN_CLASS_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include <stbi/stb_image.h>

#include <string>
#include <iostream>

#include "shader.h"
#include "Camera.h"

class Terrain
{
public:
	Terrain();

	void initPlanar(GLuint& shader);
	void renderPlanar(GLuint& shader, Camera camera, unsigned int width, unsigned int height,
		glm::mat4 view, glm::mat4 projection, glm::vec3 lightDir);
};
#endif