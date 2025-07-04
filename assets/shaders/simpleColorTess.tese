#version 450 core

layout(triangles, equal_spacing, ccw) in;

in vec3 tcPosition[];
in vec3 tcNormal[];

out vec3 te_position_world;
out vec3 te_normal_world;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewProjMatrix;

vec3 evalPNPosition(vec3 p0, vec3 p1, vec3 p2,
                    vec3 n0, vec3 n1, vec3 n2,
                    vec3 bary) {
    // PN-Triangle control points from "Curved PN Triangles"

    // Edge points
    vec3 e0 = p0 + dot(p1 - p0, n0) * n0 / 3.0;
    vec3 e1 = p1 + dot(p2 - p1, n1) * n1 / 3.0;
    vec3 e2 = p2 + dot(p0 - p2, n2) * n2 / 3.0;

    // Center point
    vec3 averageNormal = normalize((n0 + n1 + n2) / 3.0);
    vec3 centroide = (p0 + p1 + p2) / 3.0;

    // Push midpoint outward along normal
    vec3 c = centroide + averageNormal * length(p0 - centroide) * 0.5; 
    // 
    vec3 pos =
        bary.x * bary.x * p0 +
        bary.y * bary.y * p1 +
        bary.z * bary.z * p2 +
        2.0 * bary.x * bary.y * e0 +
        2.0 * bary.y * bary.z * e1 +
        2.0 * bary.z * bary.x * e2 +
        4.0 * bary.x * bary.y * bary.z * (c - (p0 + p1 + p2) / 3.0);

    return pos;
}

void main() {
    vec3 p0 = tcPosition[0];
    vec3 p1 = tcPosition[1];
    vec3 p2 = tcPosition[2];

    vec3 n0 = normalize(tcNormal[0]);
    vec3 n1 = normalize(tcNormal[1]);
    vec3 n2 = normalize(tcNormal[2]);

    vec3 bary = gl_TessCoord;

    vec3 pos = evalPNPosition(p0, p1, p2, n0, n1, n2, bary);
    vec3 norm = normalize(bary.x * n0 + bary.y * n1 + bary.z * n2);

    vec4 worldPos = modelMatrix * vec4(pos, 1.0);
    te_position_world = worldPos.xyz;
    te_normal_world = normalize(normalMatrix * norm);
    gl_Position = viewProjMatrix * worldPos;
}
