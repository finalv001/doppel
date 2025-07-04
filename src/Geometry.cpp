/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */

#include "Geometry.h"
#include <glm/glm.hpp>

#undef min
#undef max

Geometry::Geometry(glm::mat4 modelMatrix, const GeometryData &data)
    : Geometry(modelMatrix, data, WORLD_BOTH)
{
}

Geometry::Geometry(glm::mat4 modelMatrix, const GeometryData &data, uint32_t worldMask)
    : elements{static_cast<unsigned int>(data.indices.size())}, modelMatrix{modelMatrix}, geometryData{data}, worldMask{worldMask}
{
    std::cout << "Creating VAO for Geometry: " << vao << std::endl;

    // create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    std::cout << "Generated VAO: " << vao << std::endl;

    // create positions VBO
    glGenBuffers(1, &vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, data.positions.size() * sizeof(glm::vec3), data.positions.data(), GL_STATIC_DRAW);

    // bind positions to location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // create normals VBO
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, data.normals.size() * sizeof(glm::vec3), data.normals.data(), GL_STATIC_DRAW);

    // bind normals to location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // create uvs VBO
    glGenBuffers(1, &vboUVs);
    glBindBuffer(GL_ARRAY_BUFFER, vboUVs);
    glBufferData(GL_ARRAY_BUFFER, data.uvs.size() * sizeof(glm::vec2), data.uvs.data(), GL_STATIC_DRAW);

    // bind uvs to location 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    if (data.colors.size() > 0)
    {
        glGenBuffers(1, &vboColor);
        glBindBuffer(GL_ARRAY_BUFFER, vboColor);
        glBufferData(GL_ARRAY_BUFFER, data.colors.size() * sizeof(glm::vec3), data.colors.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // bind tangents to location 4
    if (data.tangents.size() > 0)
    {
        glGenBuffers(1, &vboTangents);
        glBindBuffer(GL_ARRAY_BUFFER, vboTangents);
        glBufferData(GL_ARRAY_BUFFER, data.tangents.size() * sizeof(glm::vec3), data.tangents.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // create and bind indices VBO
    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

glm::mat4 Geometry::getModelMatrix()
{
    return modelMatrix;
}

glm::vec3 Geometry::getPosition() const
{
    return glm::vec3(modelMatrix[3]);
}

void Geometry::setModelMatrix(glm::mat4 m)
{
    modelMatrix = m;
}

Geometry::~Geometry()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vboPositions);
    glDeleteBuffers(1, &vboNormals);
    glDeleteBuffers(1, &vboUVs);
    glDeleteBuffers(1, &vboTangents);
    if (vboColor != 0)
    {
        glDeleteBuffers(1, &vboColor);
    }
    glDeleteBuffers(1, &vboIndices);
}

void Geometry::drawInWorld(uint32_t worldFlag, bool underwater)
{

    if ((worldMask & worldFlag) == 0)
        return;

    auto material = (worldFlag == WORLD_BLOOM) && !underwater ? bloomyMaterial : ditherMaterial;
    if (!material)
    {
        std::cerr << "[Geometry] Warning: no material for worldFlag " << worldFlag << "\n";
        return;
    }

    Shader *shader = material->getShader();
    if (!shader)
    {
        std::cerr << "[Geometry] Warning: material has no shader\n";
        return;
    }

    shader->use();
    // std::cout << "Drawing with shader ID: " << material->getShader()->getID() << std::endl;
    material->setUniforms();
    this->draw(shader);
}

void Geometry::draw(Shader *shader)
{
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix))));

    glBindVertexArray(vao);

    if (shader->isTessellationShader())
    {

        glPatchParameteri(GL_PATCH_VERTICES, 3);
        glDrawElements(GL_PATCHES, elements, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

void Geometry::transform(glm::mat4 transformation) { modelMatrix = transformation * modelMatrix; }

void Geometry::resetModelMatrix() { modelMatrix = glm::mat4(1); }

GeometryData Geometry::createInfinitePlane(float size)
{
    GeometryData plane;

    // Vertex-Positionen für ein großes Quad
    plane.positions = {
        {-size, 0.0f, -size},
        {size, 0.0f, -size},
        {size, 0.0f, size},
        {-size, 0.0f, size}};

    // Indizes für zwei Dreiecke (Triangle Strip)
    // Beachte, dass die Indizes hier weggelassen wurden
    plane.indices = {0, 1, 2, 2, 3, 0};

    // Normale für alle Vertices (nach oben zeigend)
    plane.normals = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}};

    plane.colors = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}};
    plane.uvs = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}};

    return plane;
}

GeometryData Geometry::createSphereGeometry(unsigned int longitudeSegments, unsigned int latitudeSegments, float radius)
{
    GeometryData data;

    data.positions.push_back(glm::vec3(0.0f, radius, 0.0f));
    data.positions.push_back(glm::vec3(0.0f, -radius, 0.0f));
    data.normals.push_back(glm::vec3(0.0f, radius, 0.0f));
    data.normals.push_back(glm::vec3(0.0f, -radius, 0.0f));
    data.uvs.push_back(glm::vec2(0.0f, 0.0f));
    data.uvs.push_back(glm::vec2(0.0f, 1.0f));

    // first and last ring
    for (unsigned int j = 0; j < longitudeSegments; j++)
    {
        data.indices.push_back(0);
        data.indices.push_back(j == longitudeSegments - 1 ? 2 : (j + 3));
        data.indices.push_back(2 + j);

        data.indices.push_back(2 + (latitudeSegments - 2) * longitudeSegments + j);
        data.indices.push_back(
            j == longitudeSegments - 1 ? 2 + (latitudeSegments - 2) * longitudeSegments
                                       : 2 + (latitudeSegments - 2) * longitudeSegments + j + 1);
        data.indices.push_back(1);
    }

    // vertices and rings
    for (unsigned int i = 1; i < latitudeSegments; i++)
    {
        float verticalAngle = float(i) * glm::pi<float>() / float(latitudeSegments);
        for (unsigned int j = 0; j < longitudeSegments; j++)
        {
            float horizontalAngle = float(j) * 2.0f * glm::pi<float>() / float(longitudeSegments);
            glm::vec3 position = glm::vec3(
                radius * glm::sin(verticalAngle) * glm::cos(horizontalAngle),
                radius * glm::cos(verticalAngle),
                radius * glm::sin(verticalAngle) * glm::sin(horizontalAngle));
            data.positions.push_back(position);
            data.normals.push_back(glm::normalize(position));
            data.uvs.push_back(glm::vec2(horizontalAngle / (2.0f * glm::pi<float>()), verticalAngle / glm::pi<float>()));

            if (i == 1)
                continue;

            data.indices.push_back(2 + (i - 1) * longitudeSegments + j);
            data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 2) * longitudeSegments : 2 + (i - 2) * longitudeSegments + j + 1);
            data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 1) * longitudeSegments : 2 + (i - 1) * longitudeSegments + j + 1);

            data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 2) * longitudeSegments : 2 + (i - 2) * longitudeSegments + j + 1);
            data.indices.push_back(2 + (i - 1) * longitudeSegments + j);
            data.indices.push_back(2 + (i - 2) * longitudeSegments + j);
        }
    }

    return data;
}

GeometryData Geometry::createCubeGeometry(float width, float height, float depth)
{
    GeometryData data;

    data.positions = {
        // front
        glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f),
        // back
        glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
        // right
        glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f),
        // left
        glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
        // top
        glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
        // bottom
        glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f)};

    // clang-format off
	data.normals = {
		// front
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		// back
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		// right
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		// left
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		// top
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		// bottom
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0)
	};

	data.uvs = {
		// front
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		// back
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		// right
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		// left
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		// top
		glm::vec2(0, 1),
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		// bottom
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1)
	};

	data.indices = {
		// front
		0, 1, 2,
		2, 3, 0,
		// back
		4, 5, 6,
		6, 7, 4,
		// right
		8, 9, 10,
		10, 11, 8,
		// left
		12, 13, 14,
		14, 15, 12,
		// top
		16, 17, 18,
		18, 19, 16,
		// bottom
		20, 21, 22, 
		22, 23, 20
};
    // clang-format on
    return data;
}

GeometryData Geometry::createPlaneGeometry(float width, float length)
{
    GeometryData data;

    float w = width / 2.0f;
    float l = length / 2.0f;

    data.positions = {
        glm::vec3(-w, 0.0f, -l),
        glm::vec3(w, 0.0f, -l),
        glm::vec3(w, 0.0f, l),
        glm::vec3(-w, 0.0f, l),
    };

    data.normals = {
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0),
    };

    data.uvs = {
        glm::vec2(0, 0),
        glm::vec2(1, 0),
        glm::vec2(1, 1),
        glm::vec2(0, 1),
    };

    data.indices = {
        0,
        1,
        2,
        2,
        3,
        0,
    };

    return data;
}

std::vector<std::vector<unsigned int>> Geometry::computeOneRingNeighbors(
    const std::vector<unsigned int> &indices,
    size_t vertexCount)
{
    std::vector<std::unordered_set<unsigned int>> neighborSets(vertexCount);

    // iterate triangles
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        neighborSets[i0].insert(i1);
        neighborSets[i0].insert(i2);

        neighborSets[i1].insert(i0);
        neighborSets[i1].insert(i2);

        neighborSets[i2].insert(i0);
        neighborSets[i2].insert(i1);
    }

    // transfer to flattened vector array
    std::vector<std::vector<unsigned int>> neighbors(vertexCount);
    for (size_t i = 0; i < vertexCount; ++i)
    {
        neighbors[i] = std::vector<unsigned int>(neighborSets[i].begin(), neighborSets[i].end());
    }

    return neighbors;
}