#version 330 

layout (location = 0) in vec3 Position;  

uniform mat4 lightSpaceMatrix;  
uniform mat4 modelMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(Position, 1.0);  
}