#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform sampler2D textureSampler;
uniform sampler2D depthMap;
uniform vec3 lightDir;
uniform vec3 viewPosition;

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

void main() {
    vec3 color = texture(textureSampler, TexCoords).rgb;
    color.g += 0.2;
    color = clamp(color, 0.0, 1.0);
    
    vec3 normal = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);  // Make sure direction is from surface to light
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 halfwayDir = normalize(lightDirNorm + viewDir);
    
    // Increase ambient contribution
    vec3 ambient = 0.3 * color;  // Was 0.2
    
    // Increase diffuse contribution
    float diff = max(dot(normal, lightDirNorm), 0.0);
    vec3 diffuse = 0.8 * diff * color;  // Was 0.5
    
    // Adjust specular
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.3 * spec * vec3(1.0);  // Was 0.2
    
    float shadow = calculateShadow(FragPosLightSpace);
    
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    
    FragColor = vec4(result, 1.0);
}