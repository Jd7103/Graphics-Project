#version 330 core

layout(location = 0) in vec3 vertexPosition;

out vec3 texCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {

	//Negative Z Position Since Cubemaps use the Opposite Handed Coordinate System to the Rest of OpenGL
	texCoords = vec3(vertexPosition.x, vertexPosition.y, -vertexPosition.z);

	vec4 pos = projection * view * vec4(vertexPosition, 1.0);

	//Z = W for Max Depth
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);

}
