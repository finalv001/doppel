#pragma once

#include "tiny_gltf.h"
#include "Geometry.h"
#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "RenderObject.h"
#include <string>
#include <vector>
#include <iostream>
#include "Physics.h"

class GLTFLoader
{
public:
    GLTFLoader(Physics &physics);
    ~GLTFLoader();

    std::shared_ptr<RenderObject> loadModel(const std::string &filePath,
        std::shared_ptr<Material> bloomyMaterial,
        std::shared_ptr<Material> ditherMaterial,
        uint32_t worldMask,
        RigidBodyType bodyType);

private:
    Physics &physics;
    /*!
     * Reads and extracts the geometry data
     */
    std::shared_ptr<Geometry> processGLTFData(tinygltf::Model model);

    void generateTangents(GeometryData& data);
};
