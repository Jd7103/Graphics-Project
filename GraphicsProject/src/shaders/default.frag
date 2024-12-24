#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

in vec4 FragPosLightSpace;

uniform sampler2D textureSampler;
uniform sampler2D depthMap;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;
uniform sampler2D texture_height;


uniform vec3 lightDir;
uniform vec3 viewPosition;
uniform vec3 diffuseColour;
uniform vec3 lightPosition;

uniform int useTexture;

float shadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    float bias = max(1e-4 * (1.0 - dot(normal, lightDir)), 1e-4);

    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
    vec3 colour = diffuseColour;
    vec3 specularColour = vec3(0.3);
    vec3 normalMap = vec3(0.0);
    float heightMap = 0.0;

    if (useTexture == 1) {
        colour = texture(textureSampler, TexCoords).rgb;
    }
    else if (useTexture == 2) {
        colour = texture(texture_diffuse, TexCoords).rgb;

        normalMap = texture(texture_normal, TexCoords).rgb;
        normalMap = normalize(normalMap * 2.0 - 1.0);

        specularColour = texture(texture_specular, TexCoords).rgb;

        heightMap = texture(texture_height, TexCoords).r;
    }
        
        

    vec3 ambient = 0.3 * colour * (1.0 + heightMap * 0.2);

    vec3 lightDirNormal = normalize(-lightDir);
    vec3 normal = normalize(Normal);

    if (useTexture == 2) {
        normal = normalMap;
    }

    float diff = max(dot(lightDirNormal, normal), 0.0);
    vec3 diffuse = diff * colour * 0.5;

    vec3 viewDir = normalize(viewPosition - FragPos);

    vec3 halfwayDir = normalize(lightDirNormal + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = specularColour * spec;
    
    float shadow = shadowCalculation(FragPosLightSpace);
    colour = (ambient + (1.0 - shadow) * (diffuse + specular)) * colour;

    colour = colour/(colour + vec3(1.0));
    colour = pow(colour, vec3(1.0 / 2.2));

    colour = min(colour, vec3(1.0));

    FragColor = vec4(colour, 1.0);
    
}
