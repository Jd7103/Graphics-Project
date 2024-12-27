/*
#version 330 core
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 viewDir = normalize(-FragPos);
    vec3 normal = normalize(Normal);
    
    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = vec3(ambientStrength);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.7);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * vec3(0.5);
    
    // Fresnel effect for water-like appearance
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 2.0);
    
    // Water color
    vec3 baseColor = mix(
        vec3(0.1, 0.4, 0.8),  // Deep water color
        vec3(0.5, 0.7, 1.0),  // Surface color
        fresnel
    );
    
    // Final color
    vec3 result = (ambient + diffuse + specular) * baseColor;
    
    // Add transparency
    float alpha = mix(0.7, 0.9, fresnel);
    
    FragColor = vec4(result, alpha);
}
*/

#version 330 core
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

void main() {
    vec3 viewDir = normalize(-FragPos);
    vec3 normal = normalize(Normal);
    float viewAngle = 1.0 - abs(dot(normal, viewDir));
    
    vec3 baseColor = vec3(0.1, 0.4, 0.8);
    vec3 surfaceColor = vec3(0.5, 0.7, 1.0);
    
    float sigma = mix(0.6, 1.2, viewAngle);
    float blur = 0.0;
    float totalWeight = 0.0;
    
    for(int i = -4; i <= 4; i++) {
        for(int j = -4; j <= 4; j++) {
            float theta = float(i) * mix(0.15, 0.3, viewAngle);
            float phi = float(j) * mix(0.15, 0.3, viewAngle);
            
            vec3 offsetNormal = normal;
            offsetNormal.x = normal.x * cos(theta) - normal.y * sin(theta);
            offsetNormal.y = normal.x * sin(theta) + normal.y * cos(theta);
            offsetNormal.z = normal.z * cos(phi);
            
            float weight = exp(-(theta * theta + phi * phi) / (2.0 * sigma * sigma));
            float dist = 1.0 - dot(offsetNormal, normal);
            blur += smoothstep(1.0, 0.0, dist) * weight;
            totalWeight += weight;
        }
    }
    blur /= totalWeight;
    
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 1.5);
    vec3 waterColor = mix(baseColor, surfaceColor, fresnel);
    
    // Softer top-focused lighting
    vec3 lightDir = normalize(vec3(0.2, 1.0, 0.2));
    float diffuse = pow(max(dot(normal, lightDir), 0.0), 0.7);
    diffuse = mix(0.3, 1.0, diffuse);
    
    vec3 finalColor = mix(waterColor * diffuse, surfaceColor, blur * 0.5);
    
    float alpha = mix(0.4, 0.2, pow(viewAngle, 0.5));
    alpha *= mix(0.5, 0.9, fresnel);
    alpha *= blur;
    
    FragColor = vec4(finalColor, alpha);
}

/*
#version 330 core

// Inputs
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

// Hardcoded constants
const vec2 screenSize = vec2(1920.0, 1080.0); // Screen size in pixels
const float blurRadius = 25.0;              // Radius of the blur effect
const vec4 particleColor = vec4(0.1, 0.4, 0.8, 0.7); // Base color (RGB) and alpha

void main() {
    // Calculate UV coordinates for screen space
    vec2 uv = gl_FragCoord.xy / screenSize;

    // Initialize accumulators for color and weights
    vec3 colorSum = vec3(0.0);
    float alphaSum = 0.0;
    float weightSum = 0.0;

    // Define the number of samples for the blur effect
    int samples = 8; // Adjust for quality vs. performance

    // Perform a sampling loop
    for (int x = -samples; x <= samples; x++) {
        for (int y = -samples; y <= samples; y++) {
            // Calculate the offset in UV coordinates
            vec2 offset = vec2(x, y) * (blurRadius / screenSize);

            // Simulate particle color contribution from surrounding pixels
            float weight = exp(-(x * x + y * y) / (2.0 * blurRadius * blurRadius));
            colorSum += particleColor.rgb * weight;
            alphaSum += particleColor.a * weight;
            weightSum += weight;
        }
    }

    // Normalize by the total weight
    vec3 blurredColor = colorSum / weightSum;
    float blurredAlpha = alphaSum / weightSum;

    // Output the blurred particle color
    FragColor = vec4(blurredColor, blurredAlpha);
}
*/
