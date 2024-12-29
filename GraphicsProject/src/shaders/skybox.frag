#version 330 core

in vec3 texCoords;

uniform samplerCube textureSampler;

out vec4 fragColour;

void main() {

	fragColour = texture(textureSampler, texCoords);
	
}
