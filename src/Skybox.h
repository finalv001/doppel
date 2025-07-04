#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Shader.h"
class Skybox
{
public:
  Skybox();
  ~Skybox();
  void draw(const glm::mat4 &viewProjMatrix, bool inBloomyWorld);

private:
  GLuint vao, vbo;
  Shader skyShader;
};
