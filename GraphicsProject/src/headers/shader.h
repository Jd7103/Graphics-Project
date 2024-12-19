#ifndef _SHADER_H_
#define _SHADER_H_

#include <glad/glad.h>
#include <string>

GLuint LoadShadersFromFile(const char *vertex_file_path, const char *fragment_file_path);

GLuint LoadShadersFromString(std::string VertexShaderCode, std::string FragmentShaderCode);

#endif
