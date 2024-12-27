
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in mat4 instanceMatrix;

uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(instanceMatrix * vec4(aPos, 1.0));
    Normal = normalize(aPos);  // For a unit sphere, position == normal
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

