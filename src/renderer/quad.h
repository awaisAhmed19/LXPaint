#pragma once
#include <glad/glad.h>

class Quad {
public:
  GLuint VAO;

  Quad();
  void draw();
};
