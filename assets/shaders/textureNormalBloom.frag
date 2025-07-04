#version 450 core

layout(location = 0) in vec3 v_position_world;
layout(location = 1) in vec3 v_normal_world;
layout(location = 2) in vec2 v_texcoord;
layout(location = 3) in vec3 v_color;
layout(location = 4) in vec3 v_tangent_world;

layout(location = 0)  out vec4 FragColor;
layout(location = 1)  out vec4 BrightColor;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D shadowMap;

uniform mat4 lightSpaceMatrix;

uniform vec3 materialCoefficients;  // (ambient, diffuse, specular)
uniform float specularAlpha;
uniform vec3 camera_world;
uniform vec3 materialColor;
uniform int useNormalMap;

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

// Bayer 8x8 matrix for dithered shadow
const float bayerMatrix8x8[64] = float[64](
    0.0/64.0, 48.0/64.0, 12.0/64.0, 60.0/64.0, 3.0/64.0, 51.0/64.0, 15.0/64.0, 63.0/64.0,
    32.0/64.0, 16.0/64.0, 44.0/64.0, 28.0/64.0, 35.0/64.0, 19.0/64.0, 47.0/64.0, 31.0/64.0,
    8.0/64.0, 56.0/64.0, 4.0/64.0, 52.0/64.0, 11.0/64.0, 59.0/64.0, 7.0/64.0, 55.0/64.0,
    40.0/64.0, 24.0/64.0, 36.0/64.0, 20.0/64.0, 43.0/64.0, 27.0/64.0, 39.0/64.0, 23.0/64.0,
    2.0/64.0, 50.0/64.0, 14.0/64.0, 62.0/64.0, 1.0/64.0, 49.0/64.0, 13.0/64.0, 61.0/64.0,
    34.0/64.0, 18.0/64.0, 46.0/64.0, 30.0/64.0, 33.0/64.0, 17.0/64.0, 45.0/64.0, 29.0/64.0,
    10.0/64.0, 58.0/64.0, 6.0/64.0, 54.0/64.0, 9.0/64.0, 57.0/64.0, 5.0/64.0, 53.0/64.0,
    42.0/64.0, 26.0/64.0, 38.0/64.0, 22.0/64.0, 41.0/64.0, 25.0/64.0, 37.0/64.0, 21.0/64.0
);

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float calculateDitheredShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir) {
    lightSpacePos /= lightSpacePos.w;
    vec3 projCoords = lightSpacePos.xyz * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
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
    shadow = smoothstep(0.2, 0.8, shadow);

    int px = int(gl_FragCoord.x) % 8;
    int py = int(gl_FragCoord.y) % 8;
    float threshold = bayerMatrix8x8[py * 8 + px];

    if (shadow > 0.0 && shadow < 1.0) {
        shadow = smoothstep(threshold - 0.1, threshold + 0.1, shadow);
    }

    return shadow;
}

float getPlaneFogAmount(vec3 pos, float innerRadius, float outerRadius) {
    float xFade = clamp((abs(pos.x) - innerRadius) / (outerRadius - innerRadius), 0.0, 1.0);
    float zFade = clamp((abs(pos.z) - innerRadius) / (outerRadius - innerRadius), 0.0, 1.0);
    return max(xFade, zFade);
}
void main() {

    vec3 norm;

    if (useNormalMap == 1) {
        // TBN matrix for normal mapping
        vec3 T = normalize(v_tangent_world);
        vec3 N = normalize(v_normal_world);
        vec3 B = normalize(cross(N, T));
        mat3 TBN = mat3(T, B, N);

        // Sample and transform normal
        vec3 sampledNormal = texture(normalTexture, v_texcoord).rgb * 2.0 - 1.0;
        norm = normalize(TBN * sampledNormal);
    } else {
        norm = normalize(v_normal_world);
    }

    vec3 viewDir = normalize(camera_world - v_position_world);

    // Directional light
    vec3 lightDir = normalize(-dirL.direction);
    float diffD = max(dot(norm, lightDir), 0.0);
    vec3 diffuseD = materialCoefficients.y * diffD * dirL.color;

    vec3 reflectDirD = reflect(-lightDir, norm);
    float specD = pow(max(dot(viewDir, reflectDirD), 0.0), specularAlpha);
    vec3 specularD = materialCoefficients.z * specD * dirL.color;

    // Point light
    vec3 pointVec = pointL.position - v_position_world;
    vec3 pointDir = normalize(pointVec);
    float dist = length(pointVec);
    float att = 1.0 / (
    pointL.attenuation.x +
    pointL.attenuation.y * dist +
    pointL.attenuation.z * dist * dist
    );

    float diffP = max(dot(norm, pointDir), 0.0);
    vec3 diffuseP = materialCoefficients.y * diffP * pointL.color * att;

    vec3 reflectDirP = reflect(-pointDir, norm);
    float specP = pow(max(dot(viewDir, reflectDirP), 0.0), specularAlpha);
    vec3 specularP = materialCoefficients.z * specP * pointL.color * att;

    // Final light components
    vec3 ambient = materialCoefficients.x * dirL.color;
    vec3 diffuse = diffuseD + diffuseP;
    vec3 specular = specularD + specularP;

    vec3 albedo = texture(diffuseTexture, v_texcoord).rgb;

    // Shadowing (directional only)
    vec4 lightSpacePos = lightSpaceMatrix * vec4(v_position_world, 1.0);
    float ditheredShadow = calculateDitheredShadow(lightSpacePos, norm, lightDir);
    diffuseD *= ditheredShadow;// only directional gets shadow
    specularD *= ditheredShadow;

    // Combine shadowed and unshadowed
    diffuse = diffuseD + diffuseP;
    specular = specularD + specularP;

    // Fog
    float distance = length(camera_world - v_position_world);
    float fogStart = 0.0;
    float fogEnd = 10.0;
    float fogAmount = clamp((distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    float planeFogAmount = getPlaneFogAmount(v_position_world, 30.0, 35.0);
    float combinedFog = max(fogAmount, planeFogAmount);

    vec2 noiseUV = gl_FragCoord.xy / 512.0;
    float n = hash(noiseUV);
    vec3 fogColor = vec3(151.0 / 255.0, 154.0 / 255.0, 187.0 / 255.0) + (n - 0.5) * 0.1;

    vec3 litColor = (ambient + diffuse + specular) * albedo;
    vec3 result = mix(fogColor, litColor, 1.0 - combinedFog);
    result += (n - 0.5) * 0.015;
    result = clamp(result, 0.0, 1.0);

    FragColor = vec4(result, 1.0 - combinedFog * 0.5);
     float brightness = dot(litColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.6)
        BrightColor = vec4(litColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
