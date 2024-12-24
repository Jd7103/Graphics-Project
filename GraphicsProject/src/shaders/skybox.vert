#version 410 core

layout(location = 0) in vec3 vertexPosition;

out vec3 uv;

uniform mat4 projection;
uniform mat4 view;

void main() {
  uv = vec3(vertexPosition.x, vertexPosition.y, -vertexPosition.z);

  vec4 pos = projection * view * vec4(vertexPosition, 1.0);
  gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
}
