#ifndef SKYBOX_CLASS_H
#define SKYBOX_CLASS_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>

#include <string>
#include <iostream>

#include "shader.h"
#include "Camera.h"

class Skybox
{
public:
	Skybox();

	void SkyboxInit();
	void renderSkybox(Camera camera, unsigned int width, unsigned int height);
};
#endif