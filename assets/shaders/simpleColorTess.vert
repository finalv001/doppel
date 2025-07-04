#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 vPosition;
out vec3 vNormal;

void main() {
    vPosition = position;
    vNormal = normal;
}