#version 330 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoords;

out vec2 v_TexCoords;

void main()
{
    v_TexCoords = TexCoords;
    gl_Position = vec4(Position.xy, 0.0, 1.0);
}
