#version 330

// Eingabe-Attribute (aus dem VBO)
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

// Weitergabe an den Fragment-Shader
out vec3 position_world;
out vec3 normal_world;

// Uniforms für Transformationen
uniform mat4 modelMatrix;     
uniform mat4 viewProjMatrix;   
uniform mat3 normalMatrix;     

void main() {
    // **Normale transformieren** (falls das Modell skaliert wurde)
    normal_world = normalize(normalMatrix * normal);
    // **Position in Welt-Koordinaten berechnen**
    vec4 position_world_ = modelMatrix * vec4(position, 1.0);
    position_world = position_world_.xyz;

    // **Endgültige Position setzen**
    gl_Position = viewProjMatrix * position_world_;
}
