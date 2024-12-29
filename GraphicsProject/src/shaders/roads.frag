#version 330 core

out vec4 fragColour;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform sampler2D roadSampler;
uniform sampler2D pathSampler;
uniform sampler2D depthMap;


uniform vec3 lightDir;
uniform vec3 viewPosition;
uniform vec3 diffuseColour;

float chunkSize = 1000.0f; 
float roadWidth = 100.0f;

struct RoadInfo {

    bool isVertical;
    bool isHorizontal;
    float verticalBlend;
    float horizontalBlend;
    bool isIntersection;

};

RoadInfo calculateRoadMask() {

    RoadInfo road;
    
    //Convert to Coordinates Within Chunk
    vec2 chunkLocalPos = vec2(mod(abs(FragPos.x), chunkSize), mod(abs(FragPos.z), chunkSize));
    
    //Distance from Horizontal and Vertical Grid Lines (i.e. Roads)
    float distanceFromVertical = abs(chunkLocalPos.x - chunkSize * 0.5);
    float distanceFromHorizontal = abs(chunkLocalPos.y - chunkSize * 0.5);
    
    //Smooth Transition Between Roads and Sidewalks
    road.verticalBlend = smoothstep(roadWidth * 0.5, roadWidth * 0.45, distanceFromVertical);
    road.horizontalBlend = smoothstep(roadWidth * 0.5, roadWidth * 0.45, distanceFromHorizontal);
    
    //Determine Road Type (Horizontal or Vertical)
    road.isVertical = road.verticalBlend > 0.0;
    road.isHorizontal = road.horizontalBlend > 0.0;
    
    //Road is on an Intersection if it is Both Horizontal and Vertical
    road.isIntersection = road.isVertical && road.isHorizontal;
    
    return road;

}

vec3 sampleRoadIntersection() {

    //Intersections Coloured as the Mode Colour of 9 Samples in the Road Texture
    vec2 samplePoints[9] = vec2[](

        vec2(0.1, 0.5), vec2(0.3, 0.5), vec2(0.5, 0.5),
        vec2(0.7, 0.5), vec2(0.5, 0.1), vec2(0.5, 0.3),
        vec2(0.5, 0.7), vec2(0.5, 0.9), vec2(0.5, 0.5)

    );
    
    vec3 avgColor = vec3(0.0);
    for(int i = 0; i < 9; i++) {

        avgColor += texture(roadSampler, samplePoints[i]).rgb;
    }

    return avgColor / 9.0;

}

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

    RoadInfo road = calculateRoadMask();
    
    vec2 horizontalUV = vec2(FragPos.x / roadWidth, FragPos.z / (roadWidth * 0.5));
    vec2 verticalUV = vec2(FragPos.z / roadWidth, FragPos.x / (roadWidth * 0.5));
    
    vec3 verticalRoadColor = texture(roadSampler, verticalUV).rgb;
    vec3 horizontalRoadColor = texture(roadSampler, horizontalUV).rgb;
    vec3 intersectionColor = sampleRoadIntersection();
    
    //Assign Colour based on Texture Type
    vec3 textureColour;
    if(road.isIntersection) {

        textureColour = intersectionColor;

    } else if(road.isVertical) {

        textureColour = verticalRoadColor;

    } else if(road.isHorizontal) {

        textureColour = horizontalRoadColor;

    } else {

        //If Not Horizontal, Vertical, or Intersection, then it is a Footpath/Sidewalk
        textureColour = texture(pathSampler, TexCoords).rgb;

    }
             
             
    //Blinn Phong Lighting
    
    //Calculate and Normalise Vectors
    vec3 normal = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 halfwayDir = normalize(lightDirNorm + viewDir);
    
    //Ambient Light
    vec3 ambient = 0.3 * textureColour;
    
    //Diffuse Light 
    float diff = max(dot(normal, lightDirNorm), 0.0);
    vec3 diffuse = 0.5 * diff * textureColour;
    
    //Specular Light
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.2 * spec * vec3(1.0);
    
    //Combine and Add Shadows
    float shadow = calculateShadow(FragPosLightSpace);
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
        
    fragColour = vec4(result, 1.0);

}