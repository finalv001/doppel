#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;
uniform mat3 normalMatrix;  

uniform mat4 lightSpaceMatrix;

out vec3 position_world;
out vec3 normal_world;
out vec2 uv_coords;

void main() {
    
    normal_world = normalize(mat3(normalMatrix) * normal);
    
    vec4 position_world_ = modelMatrix * vec4(position, 1.0);
    position_world = position_world_.xyz;

    uv_coords = uv;

    gl_Position = viewProjMatrix * position_world_;
}