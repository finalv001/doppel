
#version 330 core
out vec4 FragColor;
  
in vec2 v_TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

void main()
{             
    vec3 hdrColor = texture(scene, v_TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, v_TexCoords).rgb;
    hdrColor += bloomColor;
  
    // vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    FragColor = vec4(hdrColor, 1.0);
}  