#pragma once
#include <glad/glad.h>

class Shader {
public:
  GLuint id;

  Shader(const char *vertexSrc, const char *fragmentSrc);
  void use();
};
