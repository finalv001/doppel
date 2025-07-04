#version 330

// Eingabe-Attribute (aus dem VBO)
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;


// Weitergabe an den Fragment-Shader
out vec3 position_world;
out vec3 normal_world;
out vec2 uv_coords;


// Uniforms für Transformationen
uniform mat4 modelMatrix;     
uniform mat4 viewProjMatrix;   
uniform mat3 normalMatrix;     
uniform float u_time;

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

float directionalWave(vec2 uv, vec2 dir, float speed, float frequency, float amplitude, float time) {
    float wave = sin(dot(uv, dir) * frequency + time * speed);
    return wave * amplitude;
}

void main() {
     uv_coords = uv;
    vec3 newPos = position;
    normal_world = normalize(normalMatrix * normal);
     float scale = 100.0;
    vec2 animatedUV = uv * scale + vec2(u_time * 0.05, u_time * 0.025);
    vec2 warp = vec2(
        fbm(animatedUV * 2.0 + u_time * 0.25, 6),
        fbm(animatedUV * 2.0 - u_time * 0.25, 6)
    );
    float amplitude =0.5;

    float raw = fbm(animatedUV + warp * 0.5, 6);
    float displacement = (raw - 0.5) * amplitude * 2.0;  

    vec2 dir1 = normalize(vec2(1.0, 0.5));
    vec2 dir2 = normalize(vec2(-0.6, 1.0));
    vec2 dir3 = normalize(vec2(0.3, -1.0));

    float wave1 = directionalWave(animatedUV, dir1, 1.5, 4.0, 0.1, u_time);
    float wave2 = directionalWave(animatedUV, dir2, 1.0, 6.0, 0.07, u_time);
    float wave3 = directionalWave(animatedUV, dir3, 1.2, 8.0, 0.05, u_time);

    float totalWave = wave1 + wave2 + wave3;
    newPos.y += normalize(normal).y * displacement * totalWave * amplitude;
    
    // **Position in Welt-Koordinaten berechnen**
    vec4 position_world_ = modelMatrix * vec4(newPos, 1.0);
    position_world = position_world_.xyz;

    // **Endgültige Position setzen**
    gl_Position = viewProjMatrix * position_world_;
}
