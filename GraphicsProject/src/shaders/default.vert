#version 330 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = vertexTexCoords;    
    FragPos = vec3(model * vec4(vertexPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * vertexNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}