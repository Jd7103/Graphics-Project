#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stbi/stb_image_write.h>

#include "Shader.h"
#include "Camera.h"
#include "Skybox.h"
#include "Terrain.h"
#include "Generator.h"
#include "Illumination.h"
#include "SPH.h"
#include "Spawn.h"
#include "Boid.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void initDepthFBO();
static void saveDepthTexture(GLuint fbo, std::string filename);

