#version 450 core

layout(vertices = 3) out;

in vec3 vPosition[];
in vec3 vNormal[];

out vec3 tcPosition[];
out vec3 tcNormal[];

uniform int u_SubdivisionLevel;

void main() {
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];

    if (gl_InvocationID == 0) {
        float level = 6;
        //  pow(2.0, float(u_SubdivisionLevel))
        gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
        gl_TessLevelInner[0] = level;
    }
}