#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoords;
// Instance matrix (takes locations 7,8,9,10)
layout (location = 7) in mat4 instanceMatrix;

out vec2 TexCoords;
out vec3 FragPos;
out vec4 FragPosLightSpace;
out vec3 Normal;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    TexCoords = vertexTexCoords;    
    FragPos = vec3(instanceMatrix * vec4(vertexPosition, 1.0));
    Normal = mat3(transpose(inverse(instanceMatrix))) * vertexNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}