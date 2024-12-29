#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 7) in mat4 instanceMatrix;

uniform mat4 lightSpaceMatrix;

void main()
{

    gl_Position = lightSpaceMatrix * instanceMatrix * vec4(vertexPosition, 1.0);

}