#version 330 core
in vec3 direction;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;  

uniform vec3 fogColor;
uniform vec3 skyColor;
uniform bool addNoise;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main()
{
    vec3 dir = normalize(direction);
    float t = smoothstep(0.0, 0.6, dir.y); 
    vec3 color = mix(fogColor, skyColor, t);

    if(addNoise){
        vec2 noiseUV = gl_FragCoord.xy / 512.0; 
        float n = hash(noiseUV);
        color += (n - 0.5) * 0.1; 
    }   

    FragColor = vec4(color, 1.0);
     BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
