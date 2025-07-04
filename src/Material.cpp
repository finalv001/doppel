/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */
#include "Material.h"

/* --------------------------------------------- */
// Base material
/* --------------------------------------------- */

Material::Material(std::shared_ptr<Shader> shader, glm::vec3 color, glm::vec3 materialCoefficients, float alpha)
    : _shader(shader)
    , _color(color)
    , _materialCoefficients(materialCoefficients)
    , _alpha(alpha) {}

Material::Material(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float alpha)
    : _shader(shader)
    , _materialCoefficients(materialCoefficients)
    , _alpha(alpha) {}

Material::~Material() {}

Shader* Material::getShader() { return _shader.get(); }

void Material::setUniforms() {
    _shader->setUniform("materialColor", _color);
    _shader->setUniform("materialCoefficients", _materialCoefficients);
    _shader->setUniform("specularAlpha", _alpha);
}

/* --------------------------------------------- */
// Texture material
/* --------------------------------------------- */

TextureMaterial::TextureMaterial(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float alpha, std::shared_ptr<Texture> diffuseTexture)
    : Material(shader, materialCoefficients, alpha)
    , _diffuseTexture(diffuseTexture) {}

    TextureMaterial::TextureMaterial(std::shared_ptr<Shader> shader,
        glm::vec3 materialCoefficients,
        float alpha,
        std::shared_ptr<Texture> diffuseTexture,
        std::shared_ptr<Texture> normalMapTexture)
: Material(shader, glm::vec3(1.0f), materialCoefficients, alpha), 
_diffuseTexture(diffuseTexture),
_normalMapTexture(normalMapTexture) {}

TextureMaterial::~TextureMaterial() {}

void TextureMaterial::setUniforms() {
    Material::setUniforms();

    if (_diffuseTexture) {
        // std::cout << "  → Binding diffuse texture ID: " << _diffuseTexture->getID() << " to unit 0" << std::endl;
        _diffuseTexture->bind(0);
        _shader->setUniform("diffuseTexture", 0);
    }

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind normal map texture if present
    if (_normalMapTexture) {
        // std::cout << "  → Binding normal texture ID: " << _normalMapTexture->getID() << " to unit 1" << std::endl;
        _normalMapTexture->bind(1);
        _shader->setUniform("normalTexture", 1);
    }
}

