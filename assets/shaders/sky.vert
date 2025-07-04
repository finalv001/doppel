#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 viewProjMatrix; 
  
out vec3 direction;

void main()
{
    direction = position;
    gl_Position = viewProjMatrix * vec4(position, 1.0);
}
