#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 v_position_world;
layout(location = 1) out vec3 v_normal_world;

uniform mat4 modelMatrix;     
uniform mat4 viewProjMatrix;   
uniform mat3 normalMatrix;     

void main() {
    vec4 pos_world = modelMatrix * vec4(position, 1.0);
    v_position_world = pos_world.xyz;
    v_normal_world = normalize(normalMatrix * normal);

    gl_Position = viewProjMatrix * pos_world;
}