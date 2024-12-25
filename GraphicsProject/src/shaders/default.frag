#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform sampler2D textureSampler;
uniform sampler2D depthMap;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform vec3 lightDir;
uniform vec3 viewPosition;
uniform vec3 diffuseColour;
uniform vec3 lightPosition;

uniform int useTexture;

// Constants for Blinn-Phong shading
const float AMBIENT_STRENGTH = 0.05;
const float DIFFUSE_STRENGTH = 0.5;
const float SPECULAR_STRENGTH = 0.2;
const float SHININESS = 32.0;

float calculateShadow(vec4 fragPosLightSpace)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    if(currentDepth > 1.0)
        return 0.0;
    
    // Calculate bias based on surface angle relative to light
    vec3 normal = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    float bias = max(1e-4 * (1.0 - dot(normal, lightDirNorm)), 1e-4);
    
    // PCF (Percentage Closer Filtering)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    
    shadow /= 25.0; // Average over the 5x5 samples
    return shadow;
}

void main()
{
    // Determine base color
    vec3 color;
    if (useTexture == 1) {
        color = texture(textureSampler, TexCoords).rgb;
    }
    else if (useTexture == 2) {
        color = texture(texture_diffuse1, TexCoords).rgb;
    }
    else {
        color = diffuseColour;
    }
    
    // Normalize vectors
    vec3 normal = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 halfwayDir = normalize(lightDirNorm + viewDir);
    
    // Ambient
    vec3 ambient = AMBIENT_STRENGTH * color;
    
    // Diffuse
    float diff = max(dot(normal, lightDirNorm), 0.0);
    vec3 diffuse = DIFFUSE_STRENGTH * diff * color;
    
    // Specular
    float spec = pow(max(dot(normal, halfwayDir), 0.0), SHININESS);
    vec3 specular = SPECULAR_STRENGTH * spec * vec3(1.0); // White specular highlight
    
    // Calculate shadow
    float shadow = calculateShadow(FragPosLightSpace);
    
    // Combine all components
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    
    // Apply gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}