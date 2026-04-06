#include "shader.h"
#include <iostream>

static GLuint compile(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(shader, 512, NULL, info);
    std::cout << "Shader error:\n" << info << std::endl;
  }

  return shader;
}

Shader::Shader(const char *vs, const char *fs) {
  GLuint v = compile(GL_VERTEX_SHADER, vs);
  GLuint f = compile(GL_FRAGMENT_SHADER, fs);

  id = glCreateProgram();
  glAttachShader(id, v);
  glAttachShader(id, f);
  glLinkProgram(id);

  glDeleteShader(v);
  glDeleteShader(f);
}

void Shader::use() { glUseProgram(id); }
