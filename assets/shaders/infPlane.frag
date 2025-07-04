#version 330 

in vec3 position_world;
in vec3 normal_world;
in vec2 uv_coords;

uniform sampler2D shadowMap; 
uniform mat4 lightSpaceMatrix;

uniform vec3 materialCoefficients;  
uniform float specularAlpha;
uniform vec3 materialColor;
uniform bool in_bloomy_world;

struct DirectionalLight {
    vec3 color;
    vec3 direction;
};
uniform DirectionalLight dirL;

struct PointLight {
    vec3 color;
    vec3 position;
    float attenuation;
};
uniform PointLight pointL;

uniform vec3 camera_world;

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

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}


float calculateDitheredShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir) {
    // Normalisierung
    lightSpacePos /= lightSpacePos.w;
    vec3 projCoords = lightSpacePos.xyz * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 1.0;

    float bias = max(0.002 * (1.0 - dot(normal, lightDir)), 0.0003);

    // PCF 5×5
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

    // Smoothstep für weicheren visuellen Übergang
    shadow = smoothstep(0.2, 0.8, shadow);

    int px = int(gl_FragCoord.x) % 8;
    int py = int(gl_FragCoord.y) % 8;
    float threshold = bayerMatrix8x8[py * 8 + px];

    // Feines Rauschen zum Weichzeichnen
    if (shadow > 0.0 && shadow < 1.0) {
    int px = int(gl_FragCoord.x) % 8;
    int py = int(gl_FragCoord.y) % 8;
    float threshold = bayerMatrix8x8[py * 8 + px];

    shadow = smoothstep(threshold - 0.1, threshold + 0.1, shadow);
}

    return shadow;
}

float valueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for (int i = 0; i < 8; i++) {
        value += amplitude * valueNoise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
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

vec3 calculateNormalFromFBM(vec2 uv, vec3 originalNormal, float scale, float strength) {
    vec2 scaledUV = uv * scale;

    float h = fbm(scaledUV);
    float h_dx = fbm(scaledUV + vec2(0.01, 0.0));
    float h_dy = fbm(scaledUV + vec2(0.0, 0.01));

    vec3 tangent = normalize(vec3(1.0, 0.0, (h_dx - h) * strength));
    vec3 bitangent = normalize(vec3(0.0, 1.0, (h_dy - h) * strength));
    vec3 normalFromMap = normalize(cross(tangent, bitangent));

    // Blend with original normal for stability
    return normalize(mix(originalNormal, normalFromMap, 0.5));
}

void main() {
    vec3 norm = calculateNormalFromFBM(uv_coords, normal_world, 30.0, 1.0);
    // vec3 norm = normalize(normal_world);
    vec3 viewDir = normalize(camera_world - position_world);
    
    // Ambient
    vec3 ambient = materialCoefficients.x * dirL.color;
    
    // Diffuse
    vec3 lightDir = normalize(-dirL.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = materialCoefficients.y * diff * dirL.color;

    // Specular
    vec3 specularColor = vec3(1.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0),  128.0);
    vec3 specular = 0.1 * specularColor * spec;



    // Schatten mit Dithering berechnen
    vec4 lightSpacePos = lightSpaceMatrix * vec4(position_world, 1.0);
    float ditheredShadow = calculateDitheredShadow(lightSpacePos, norm, lightDir);

    // Schatten auf Diffuse anwenden
    diffuse *= ditheredShadow;
    // specular *= ditheredShadow;

    float fbmValue = fbm(uv_coords * 40.0); 
    vec3 texturedColor = materialColor * mix(0.6, 1.3, fbmValue);

    float distance = length(camera_world - position_world); 
    float fogStart = 0.0;
    float fogEnd = 10.0; // fog reaching full intensity
    float fogAmount = clamp((distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 fogColor = in_bloomy_world
    ? vec3(151.0 / 255.0, 154.0 / 255.0, 187.0 / 255.0)  
    : vec3(0.9); 

    if (in_bloomy_world) {
        vec2 noiseUV = gl_FragCoord.xy / 512.0;
        float n = hash(noiseUV);
        fogColor += (n - 0.5) * 0.1;
    }

    float planeFogAmount=getPlaneFogAmount(position_world);
    float combinedFog = max(fogAmount, planeFogAmount);

    vec3 litColor = (ambient + diffuse + specular) * texturedColor;
    vec3 result = mix(fogColor, litColor, 1.0 - combinedFog);

    FragColor = vec4(result, 1.0 - combinedFog * 0.5);
    float brightness = dot(litColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(litColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}