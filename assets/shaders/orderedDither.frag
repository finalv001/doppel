#version 330

in vec3 position_world;
in vec3 normal_world;

uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

uniform vec3 camera_world;

uniform vec3 materialColor;
uniform vec3 materialCoefficients;  // ambient, diffuse, specular
uniform float specularAlpha;
uniform bool drawBloom;

struct DirectionalLight {
    vec3 color;
    vec3 direction;
};
uniform DirectionalLight dirL;

struct PointLight {
    vec3 color;
    vec3 position;
    vec3 attenuation;
};
uniform PointLight pointL;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;  

const float bayerMatrix8x8[64] = float[64](
    0.0/64.0, 48.0/64.0, 12.0/64.0, 60.0/64.0,  3.0/64.0, 51.0/64.0, 15.0/64.0, 63.0/64.0,
    32.0/64.0, 16.0/64.0, 44.0/64.0, 28.0/64.0, 35.0/64.0, 19.0/64.0, 47.0/64.0, 31.0/64.0,
    8.0/64.0, 56.0/64.0,  4.0/64.0, 52.0/64.0, 11.0/64.0, 59.0/64.0,  7.0/64.0, 55.0/64.0,
    40.0/64.0, 24.0/64.0, 36.0/64.0, 20.0/64.0, 43.0/64.0, 27.0/64.0, 39.0/64.0, 23.0/64.0,
    2.0/64.0, 50.0/64.0, 14.0/64.0, 62.0/64.0,  1.0/64.0, 49.0/64.0, 13.0/64.0, 61.0/64.0,
    34.0/64.0, 18.0/64.0, 46.0/64.0, 30.0/64.0, 33.0/64.0, 17.0/64.0, 45.0/64.0, 29.0/64.0,
    10.0/64.0, 58.0/64.0,  6.0/64.0, 54.0/64.0,  9.0/64.0, 57.0/64.0,  5.0/64.0, 53.0/64.0,
    42.0/64.0, 26.0/64.0, 38.0/64.0, 22.0/64.0, 41.0/64.0, 25.0/64.0, 37.0/64.0, 21.0/64.0
);

float calculateDitheredShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir) {
    lightSpacePos /= lightSpacePos.w;
    vec3 projCoords = lightSpacePos.xyz * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.z < 0.0 ||
        projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 1.0;

    float bias = max(0.002 * (1.0 - dot(normal, lightDir)), 0.0003);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    const int radius = 2;
    int count = 0;

    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(shadowMap, projCoords.xy + offset).r;
            shadow += projCoords.z > closestDepth + bias ? 0.0 : 1.0;
            count++;
        }
    }

    shadow /= float(count);

    int px = int(gl_FragCoord.x) % 8;
    int py = int(gl_FragCoord.y) % 8;
    float threshold = bayerMatrix8x8[py * 8 + px];

    if (shadow > 0.0 && shadow < 1.0) {
        shadow = smoothstep(threshold - 0.1, threshold + 0.1, shadow);
    }

    return shadow;
}

// dither func
vec3 orderedDither(float lum) {
    int x = int(gl_FragCoord.x) % 8;
    int y = int(gl_FragCoord.y) % 8;
    float threshold = bayerMatrix8x8[y * 8 + x];
    float ditherStrength = 0.2; 
    return lum < threshold + ditherStrength ? vec3(0.05) : vec3(1.0);
}

float getPlaneFogAmount(vec3 pos) {
    float xNeg = -3.0;
    float xPos = 30.0;
    float zNeg = -10.0;
    float zPos = 30.0;
    float fadingDistance = 5.0;

    float xFade;
    if (pos.x < 0.0) {
        float start = xNeg;
        float end = xNeg - fadingDistance;
        xFade = clamp((pos.x - start) / (end - start), 0.0, 1.0);
    } else {
        float start = xPos;
        float end = xPos + fadingDistance;
        xFade = clamp((pos.x - start) / (end - start), 0.0, 1.0);
    }

    float zFade;
    if (pos.z < 0.0) {
        float start = zNeg;
        float end = zNeg - fadingDistance;
        zFade = clamp((pos.z - start) / (end - start), 0.0, 1.0);
    } else {
        float start = zPos;
        float end = zPos + fadingDistance;
        zFade = clamp((pos.z - start) / (end - start), 0.0, 1.0);
    }

    return max(xFade, zFade);
}
void main() {
    vec3 norm = normalize(normal_world);
    vec3 viewDir = normalize(camera_world - position_world);
    vec3 lightDir = normalize(-dirL.direction);

    // Lighting
    vec3 ambient = materialCoefficients.x * dirL.color;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = materialCoefficients.y * diff * dirL.color;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularAlpha);
    vec3 specular = materialCoefficients.z * spec * vec3(1.0);

    // Shadowing
    float shadow = calculateDitheredShadow(lightSpaceMatrix * vec4(position_world, 1.0), norm, lightDir);
    diffuse *= shadow;

    // Final lit color before fog
    vec3 litColor = (ambient + diffuse + specular) * materialColor;

    // Fog
    float fogDistance = length(camera_world - position_world);
    float fogStart = 3.0;
    float fogEnd = 8.0;
    float fogAmount = clamp((fogDistance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    float planeFogAmount = getPlaneFogAmount(position_world);
    float combinedFog = max(fogAmount, planeFogAmount);
    vec3 fogColor = vec3(0.9);
    vec3 foggedColor = mix(litColor, fogColor, combinedFog);

    float lum = dot(vec3(0.2126, 0.7152, 0.0722), foggedColor);
    vec3 dithered = orderedDither(lum); 
    vec3 finalColor = mix(fogColor, dithered, 1.0 - combinedFog);

    FragColor = vec4(finalColor, 1.0 - combinedFog * 0.5);
    
     float brightness = dot(dithered.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.2 && drawBloom)
        BrightColor = vec4(dithered.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
