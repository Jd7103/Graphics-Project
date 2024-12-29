#version 330 core

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() {

    FragPos = vec3(model * vec4(vertexPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * vertexNormal;
    TexCoords = vertexTexCoords;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);

}