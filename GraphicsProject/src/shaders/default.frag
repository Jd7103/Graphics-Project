#version 330 core

out vec4 fragColour;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform sampler2D textureSampler;
uniform sampler2D depthMap;
uniform sampler2D texture_diffuse1;

uniform vec3 lightDir;
uniform vec3 viewPosition;
uniform vec3 diffuseColour;
uniform vec3 lightPosition;

uniform int useTexture;

float calculateShadow(vec4 fragPosLightSpace) {

    //Perspective Divison
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    //Convert to Normalised Device Coordinates (NDC)
    projCoords = projCoords * 0.5 + 0.5;
    
    //Sample Depth Value of Current Fragment
    float currentDepth = projCoords.z;
    
    if (currentDepth > 1.0) {
        return 0.0;
    }

    //Bias Term Calculation
    vec3 normal = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    float bias = max(1e-4 * (1.0 - dot(normal, lightDirNorm)), 1e-4);
    
    // PCF (Percentage Closer Filtering)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    
    //Accumulate Shadow Value from 25 Samples
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    
    shadow /= 25.0;
    return shadow;

}

void main() {

    // Determine base color
    vec3 colour;
    if (useTexture == 1) {
        colour = texture(textureSampler, TexCoords).rgb;
    }
    else if (useTexture == 2) {
        colour = texture(texture_diffuse1, TexCoords).rgb;
    }
    else {
        colour = diffuseColour;
    }
    
    //Calculate and Normalise vectors
    vec3 normal = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 halfwayDir = normalize(lightDirNorm + viewDir);
    
    //Ambient Light
    vec3 ambient = 0.2 * colour;
    
    //Diffuse Light
    float diff = max(dot(normal, lightDirNorm), 0.0);
    vec3 diffuse = 0.5 * diff * colour;
    
    //Specular Light
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.6 * spec * vec3(1.0);
    
    //Combine and Add Shadows
    float shadow = calculateShadow(FragPosLightSpace);
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    
    fragColour = vec4(result, 1.0);
}