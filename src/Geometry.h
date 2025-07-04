/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */
#pragma once

#include "Material.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <unordered_set>

enum WorldMask : uint32_t
{
  WORLD_NONE = 0,
  WORLD_BLOOM = 1 << 0,
  WORLD_DITHER = 1 << 1,
  WORLD_BOTH = WORLD_BLOOM | WORLD_DITHER,
  WORLD_PLAYER = 1 << 2,
  WORLD_REMOTE = 1 << 3,
  WORLD_STATIC = 1 << 4
};

/*!
 * Stores all data for a geometry object
 */
struct GeometryData
{
  /*!
   * Vertex positions
   */
  std::vector<glm::vec3> positions;
  /*!
   * Geometry indices
   */
  std::vector<unsigned int> indices;

  /*!
   * Vertex colors
   */
  std::vector<glm::vec3> colors;
  /*!
   * Vertex normals
   */
  std::vector<glm::vec3> normals;
  /*!
   * Vertex UV coordinates
   */
  std::vector<glm::vec2> uvs;

  /*!
   * Vertex tangents
   */
  std::vector<glm::vec3> tangents;
};

class Geometry
{

private:
  glm::mat4 lightSpaceMatrix;
  GeometryData geometryData;
  std::shared_ptr<Material> bloomyMaterial;
  std::shared_ptr<Material> ditherMaterial;
  uint32_t worldMask = WORLD_BOTH;

protected:
  /*!
   * Vertex array object
   */
  GLuint vao;
  /*!
   * Vertex buffer object that stores the vertex positions
   */
  GLuint vboPositions;

  /*!
   * Vertex buffer object that stores the vertex color
   */
  GLuint vboColor;

  /*!
   * Vertex buffer object that stores the vertex normals
   */
  GLuint vboNormals;
  /*!
   * Vertex buffer object that stores the vertex UV coordinates
   */
  GLuint vboUVs;
  /*!
   * Vertex buffer object that stores the indices
   */
  GLuint vboIndices;

  /*!
   * Vertex buffer object that stores the tangents
   */
  GLuint vboTangents;

  /*!
   * Number of elements to be rendered
   */
  unsigned int elements;

  /*!
   * Model matrix of the object
   */
  glm::mat4 modelMatrix;

  GLuint ssboNeighbors;
  GLuint ssboNeighborOffsets;
  GLuint ssboGlobalPositions;
  GLuint ssboPatchIndices;
  GLuint ssboNeighborCounts;

public:
  /*!
   * Geometry object constructor
   * Creates VAO and VBOs and binds them
   * @param modelMatrix: model matrix of the object
   * @param data: data for the geometry object
   * @param material: material of the geometry object
   */
  Geometry(glm::mat4 modelMatrix, const GeometryData &data, uint32_t worldMask);
  Geometry(glm::mat4 modelMatrix, const GeometryData &data);
  ~Geometry();

  void setWorldMask(uint32_t mask) { worldMask = mask; }
  void setBloomyMaterial(std::shared_ptr<Material> m) { bloomyMaterial = m; }
  void setDitherMaterial(std::shared_ptr<Material> m) { ditherMaterial = m; }

  void draw();

  void Geometry::drawInWorld(uint32_t worldFlag, bool underwater);
  /*!
   * Draws the object
   * Issues a draw call
   */
  void draw(Shader *shader);

  /*!
   * Transforms the object, i.e. updates the model matrix
   * @param transformation: the transformation matrix to be applied to the object
   */
  void transform(glm::mat4 transformation);

  /*!
   * Resets the model matrix to the identity matrix
   */
  void resetModelMatrix();

  /*!
   * Returns the model matrix
   */
  glm::mat4 Geometry::getModelMatrix();

  /*!
   * Sets the model matrix
   */
  void Geometry::setModelMatrix(glm::mat4 modelMatrix);

  const GeometryData &getGeometryData() const { return geometryData; }

  glm::vec3 getPosition() const;

  void Geometry::setUniforms();
  /*!
   * Creates a cube geometry
   * @param width: width of the cube
   * @param height: height of the cube
   * @param depth: depth of the cube
   * @return all cube data
   */

  uint32_t getWorldMask() { return worldMask; };

  static GeometryData createCubeGeometry(float width, float height, float depth);

  static GeometryData createPlaneGeometry(float width, float length);

  static GeometryData createSphereGeometry(unsigned int longitudeSegments, unsigned int latitudeSegments, float radius);

  static GeometryData createInfinitePlane(float size);

  static std::vector<std::vector<unsigned int>> computeOneRingNeighbors(
      const std::vector<unsigned int> &indices,
      size_t vertexCount);
};
