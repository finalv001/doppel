#version 330 

void main()
{
    gl_FragDepth = gl_FragCoord.z;  //save depht-info in shadow map
}