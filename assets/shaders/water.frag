#version 330 

in vec3 position_world;
in vec3 normal_world;
in vec2 uv_coords;


uniform sampler2D shadowMap; 
uniform mat4 lightSpaceMatrix;

uniform vec3 materialCoefficients;  
uniform float specularAlpha;
uniform vec3 materialColor;
uniform float u_time;


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


// Funktion zur Berechnung des Schattens mit Dithering
float calculateDitheredShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir) {
    lightSpacePos /= lightSpacePos.w;
    vec3 projCoords = lightSpacePos.xyz * 0.5 + 0.5;

    float bias = max(0.0025 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    // PCF (Percentage Closer Filtering)
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(shadowMap, projCoords.xy + offset).r;
            shadow += projCoords.z > closestDepth + bias ? 0.0 : 1.0;
        }
    }
    shadow /= 7.0;

    // Dithering auf Schatten anwenden
    int x = int(gl_FragCoord.x) % 8;
    int y = int(gl_FragCoord.y) % 8;
    float threshold = bayerMatrix8x8[y * 8 + x];

    return (shadow < threshold + 0.15) ? 0.0 : 1.0;
}

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f*f*(3.0-2.0*f);

    return mix(a, b, u.x) +
           (c - a)* u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

float fbm(vec2 p, int octaves) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;

    for(int i = 0; i < 8; i++) {
        if(i >= octaves) break;
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return value;
}
void main() {
    vec3 norm = normalize(normal_world);
    vec3 viewDir = normalize(camera_world - position_world);
    
    // Ambient
    vec3 ambient = materialCoefficients.x * dirL.color;
    
    // Diffuse
    vec3 lightDir = normalize(-dirL.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = materialCoefficients.y * diff * dirL.color;

    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularAlpha);
    vec3 specular = materialCoefficients.z * spec * dirL.color;

    // Schatten mit Dithering berechnen
    vec4 lightSpacePos = lightSpaceMatrix * vec4(position_world, 1.0);
    float ditheredShadow = calculateDitheredShadow(lightSpacePos, norm, lightDir);

    // Schatten auf Diffuse anwenden
    diffuse *= ditheredShadow;
    // specular *= ditheredShadow;

    //Prozedurale Texturen 
    float scale = 5.0;
    vec2 animatedUV = uv_coords * scale + vec2(u_time * 0.05, u_time * 0.025);

    // Domain Warping (double-FBM)
    vec2 warp = vec2(
        fbm(animatedUV * 2.0 + u_time * 0.25, 6),
        fbm(animatedUV * 2.0 - u_time * 0.25, 6)
    );
    
    float pattern = fbm(animatedUV + warp * 0.5, 6);

    specular *= 1.0 + 0.5 * pattern;
    float distance = length(camera_world - position_world); 
    float fogStart = 0.0;
    float fogEnd = 10.0; // fog reaching full intensity
    float fogAmount = clamp((distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 fogColor = vec3(0.9);

   vec3 niceOrange = vec3(0.9, 0.2, 0.0);
    vec3 niceRed = vec3(0.4, 0.0, 0.0);
    vec3 baseColor = mix(niceRed, niceOrange, pattern);

    vec3 litColor = (ambient + diffuse + specular) * baseColor;
    float fresnel = pow(1.0 - dot(viewDir, norm),1.5);
   vec3 fresnelColor = vec3(1.0, 0.6, 0.3);
    specular += fresnel * 0.5;

    litColor += fresnel * fresnelColor;
    
    vec3 result = mix(fogColor, litColor, 1.0 - fogAmount);
    FragColor = vec4(result, 1.0 - fogAmount * 0.5); // subtracting fog from alpha value to blend into bg
    
    float brightness = dot(litColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.7)
        BrightColor = vec4(litColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
