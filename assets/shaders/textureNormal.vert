#version 450 core

// Inputs from VAO
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in vec3 color;
layout(location = 4) in vec3 tangent;

// Outputs to fragment shader
layout(location = 0) out vec3 v_position_world;
layout(location = 1) out vec3 v_normal_world;
layout(location = 2) out vec2 v_texcoord;
layout(location = 3) out vec3 v_color;
layout(location = 4) out vec3 v_tangent_world;

uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;
uniform mat3 normalMatrix;

void main() {
    vec4 pos_world = modelMatrix * vec4(position, 1.0);
    v_position_world = pos_world.xyz;
    v_normal_world = normalize(normalMatrix * normal);
    v_tangent_world = normalize(normalMatrix * tangent);
    v_texcoord = texcoord;
    v_color = color;

    gl_Position = viewProjMatrix * pos_world;
}
