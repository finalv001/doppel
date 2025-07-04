#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "PathUtils.h"
#include "GLTFLoader.h"

GLTFLoader::GLTFLoader(Physics &physics) : physics(physics)
{
}
GLTFLoader::~GLTFLoader() {}

std::shared_ptr<RenderObject> GLTFLoader::loadModel(const std::string &filePath, std::shared_ptr<Material> bloomyMaterial, std::shared_ptr<Material> ditherMaterial, uint32_t worldMask, RigidBodyType bodyType)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    std::string ext = filePath.substr(filePath.find("."));
    std::string absoluteFilePath = gcgFindFileInParentDir(filePath.c_str());

    bool ret = false;
    if (ext == ".gltf")
    {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, absoluteFilePath);
    }
    else if (ext == ".glb")
    {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, absoluteFilePath);
    }
    else
    {
        throw std::runtime_error("Error, unknown file extension");
    }

    if (!warn.empty())
    {
        std::cout << "Warning: " << warn << std::endl;
    }
    if (!err.empty())
    {
        throw std::runtime_error("Error: " + err);
    }
    if (!ret)
    {
        throw std::runtime_error("Failed to load GLTF file: " + filePath);
    }

    std::cout << "Loaded GLTF file: " << filePath << std::endl;

    std::shared_ptr<Geometry> geometry = processGLTFData(model);
    geometry->setWorldMask(worldMask);
    geometry->setBloomyMaterial(bloomyMaterial);
    geometry->setDitherMaterial(ditherMaterial);

    if (bodyType == RigidBodyType::NONE)
    {
        return std::make_shared<RenderObject>(geometry);
    }

    PxRigidActor *actor = physics.createMeshFromGeometry(geometry->getGeometryData(), bodyType);

    // Erstelle das RenderObject als shared_ptr
    std::shared_ptr<RenderObject> renderObj;

    if (bodyType == RigidBodyType::STATIC)
        renderObj = std::make_shared<RenderObject>(geometry, static_cast<PxRigidStatic *>(actor));
    else
        renderObj = std::make_shared<RenderObject>(geometry, static_cast<PxRigidDynamic *>(actor));

    // Setze userData hier korrekt
    if (actor)
    {
        actor->userData = renderObj.get();
    }

    return renderObj;
}

std::shared_ptr<Geometry> GLTFLoader::processGLTFData(tinygltf::Model model)
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    GeometryData data;

    for (const auto &node : model.nodes)
    {
        if (node.translation.size() == 3)
        {
            modelMatrix = glm::translate(modelMatrix, glm::vec3(
                                                          node.translation[0],
                                                          node.translation[1],
                                                          node.translation[2]));
        }
        if (node.rotation.size() == 4)
        {
            glm::quat rotation = glm::quat(
                node.rotation[3],
                node.rotation[0],
                node.rotation[1],
                node.rotation[2]);
            modelMatrix *= glm::mat4_cast(rotation);
        }
        if (node.scale.size() == 3)
        {
            modelMatrix = glm::scale(modelMatrix, glm::vec3(
                                                      node.scale[0],
                                                      node.scale[1],
                                                      node.scale[2]));
        }

        if (node.mesh >= 0)
        {
            const auto &mesh = model.meshes[node.mesh];
            for (const auto &primitive : mesh.primitives)
            {
                unsigned int indexOffset = static_cast<unsigned int>(data.positions.size());

                // POSITION
                if (primitive.attributes.find("POSITION") != primitive.attributes.end())
                {
                    const auto &accessor = model.accessors[primitive.attributes.at("POSITION")];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto &buffer = model.buffers[bufferView.buffer];
                    size_t stride = accessor.ByteStride(bufferView) ? accessor.ByteStride(bufferView) : sizeof(float) * 3;
                    const uint8_t *dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                    for (size_t i = 0; i < accessor.count; ++i)
                    {
                        const float *positions = reinterpret_cast<const float *>(dataPtr + i * stride);
                        data.positions.push_back(glm::vec3(positions[0], positions[1], positions[2]));
                    }
                }

                // NORMAL
                if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    const auto &accessor = model.accessors[primitive.attributes.at("NORMAL")];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto &buffer = model.buffers[bufferView.buffer];
                    size_t stride = accessor.ByteStride(bufferView) ? accessor.ByteStride(bufferView) : sizeof(float) * 3;
                    const uint8_t *dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                    for (size_t i = 0; i < accessor.count; ++i)
                    {
                        const float *normals = reinterpret_cast<const float *>(dataPtr + i * stride);
                        data.normals.push_back(glm::vec3(normals[0], normals[1], normals[2]));
                    }
                }

                // TEXCOORD_0
                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    const auto &accessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto &buffer = model.buffers[bufferView.buffer];
                    size_t stride = accessor.ByteStride(bufferView) ? accessor.ByteStride(bufferView) : sizeof(float) * 2;
                    const uint8_t *dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                    for (size_t i = 0; i < accessor.count; ++i)
                    {
                        const float *uvs = reinterpret_cast<const float *>(dataPtr + i * stride);
                        data.uvs.push_back(glm::vec2(uvs[0], uvs[1]));
                    }
                }

                // INDICES
                if (primitive.indices >= 0)
                {
                    const auto &accessor = model.accessors[primitive.indices];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto &buffer = model.buffers[bufferView.buffer];
                    const uint8_t *dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        for (size_t i = 0; i < accessor.count; ++i)
                        {
                            uint16_t idx = *reinterpret_cast<const uint16_t *>(dataPtr + i * sizeof(uint16_t));
                            data.indices.push_back(indexOffset + idx);
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        for (size_t i = 0; i < accessor.count; ++i)
                        {
                            uint32_t idx = *reinterpret_cast<const uint32_t *>(dataPtr + i * sizeof(uint32_t));
                            data.indices.push_back(indexOffset + idx);
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        for (size_t i = 0; i < accessor.count; ++i)
                        {
                            uint8_t idx = *reinterpret_cast<const uint8_t *>(dataPtr + i * sizeof(uint8_t));
                            data.indices.push_back(indexOffset + idx);
                        }
                    }
                    else
                    {
                        std::cerr << "Unsupported index type: " << accessor.componentType << std::endl;
                    }
                }

                if (data.uvs.size() < data.positions.size())
                {
                    data.uvs.resize(data.positions.size(), glm::vec2(0.0f));
                }

                if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                {
                    std::cerr << "Warning: Non-triangle primitive skipped." << std::endl;
                    continue;
                }
            }
        }
    }

    // Generate tangents once all data is gathered
    if (!data.positions.empty() && !data.uvs.empty() && !data.indices.empty())
    {
        generateTangents(data);
        std::cout << "Generated tangents: " << data.tangents.size() << std::endl;
    }

    return std::make_shared<Geometry>(modelMatrix, data);
}

// https://www.khronos.org/files/gltf20-reference-guide.pdf

// std::shared_ptr<Material> material;
// std::shared_ptr<Shader> simpleDitherShader = std::make_shared<Shader>("assets/shaders/orderedDither.vert", "assets/shaders/orderedDither.frag");
// glm::vec3 color(1.0f, 1.0f, 1.0f);

// if (primitive.material >= 0)
// {
//     const auto &gltfMaterial = model.materials[primitive.material];

//     if (gltfMaterial.pbrMetallicRoughness.baseColorFactor.size() == 4)
//     {
//         color = glm::vec3(
//             gltfMaterial.pbrMetallicRoughness.baseColorFactor[0],
//             gltfMaterial.pbrMetallicRoughness.baseColorFactor[1],
//             gltfMaterial.pbrMetallicRoughness.baseColorFactor[2]);
//     }

//     std::shared_ptr<Texture> texture;
//     if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0)
//     {
//         int textureIndex = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
//         const auto &gltfTexture = model.textures[textureIndex];
//         const auto &gltfImage = model.images[gltfTexture.source];
//         texture = std::make_shared<Texture>(gltfImage.uri);
//     }

//     // if (texture)
//     // {
//     //     material = std::make_shared<TextureMaterial>(simpleDitherShader, color, glm::vec3(0.3f, 0.5f, 1.0f), texture);
//     // }
//     // else
//     // {
//     // material = std::make_shared<Material>(simpleDitherShader, glm::vec3(0.0f, 0.5f, 0.9f), glm::vec3(0.3f, 0.5f, 1.0f), 2.0f);
//     // }
// }
// else
// {
//     material = std::make_shared<Material>(simpleDitherShader, glm::vec3(0.0f, 0.5f, 0.9f), glm::vec3(0.3f, 0.5f, 1.0f), 2.0f);
// }

void GLTFLoader::generateTangents(GeometryData &data)
{
    data.tangents.resize(data.positions.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < data.indices.size(); i += 3)
    {
        uint32_t i0 = data.indices[i];
        uint32_t i1 = data.indices[i + 1];
        uint32_t i2 = data.indices[i + 2];

        const glm::vec3 &p0 = data.positions[i0];
        const glm::vec3 &p1 = data.positions[i1];
        const glm::vec3 &p2 = data.positions[i2];

        const glm::vec2 &uv0 = data.uvs[i0];
        const glm::vec2 &uv1 = data.uvs[i1];
        const glm::vec2 &uv2 = data.uvs[i2];

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float f = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        if (f == 0.0f)
            f = 1.0f;
        float r = 1.0f / f;

        glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * r;

        data.tangents[i0] += tangent;
        data.tangents[i1] += tangent;
        data.tangents[i2] += tangent;
    }

    for (auto &t : data.tangents)
    {
        if (glm::length(t) > 0.0f)
            t = glm::normalize(t);
        else
            t = glm::vec3(1.0f, 0.0f, 0.0f); // fallback tangent
    }
}