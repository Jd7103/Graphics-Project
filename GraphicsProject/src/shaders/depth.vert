#version 330 core

layout (location = 0) in vec3 vertexPosition;
// Instance matrix (takes locations 7,8,9,10)
layout (location = 7) in mat4 instanceMatrix;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    // Use instance matrix if instanced, otherwise use model matrix
    gl_Position = lightSpaceMatrix * instanceMatrix * vec4(vertexPosition, 1.0);
}