in vec3 uv;

uniform samplerCube textureSampler;

out vec4 finalColor;

void main()
{
	finalColor = texture(textureSampler, uv);
}
