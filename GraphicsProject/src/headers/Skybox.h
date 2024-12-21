#ifndef SKYBOX_CLASS_H
#define SKYBOX_CLASS_H

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

class Skybox
{
public:
	Skybox();

	void init();
	void render(Camera camera, unsigned int width, unsigned int height, glm::mat4 view, glm::mat4 projection);
	void deleteBuffers();
};
#endif