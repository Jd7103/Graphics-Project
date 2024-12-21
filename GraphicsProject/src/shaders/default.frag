#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D textureSampler;
uniform vec3 diffuseColor;
uniform int useTexture;
uniform vec3 lightDir;
uniform vec3 viewPos;

void main()
{
    vec3 colour = (useTexture == 1) ? texture(textureSampler, TexCoords).rgb : diffuseColor;

    vec3 ambient = 0.05 * colour;

    vec3 lightDirNormal = normalize(-lightDir);
    vec3 normal = normalize(Normal);
    float diff = max(dot(lightDirNormal, normal), 0.0);
    vec3 diffuse = diff * colour;

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 halfwayDir = normalize(lightDirNormal + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.3) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
