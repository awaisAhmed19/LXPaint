// OpenGL loader (must come first)
#include <glad/glad.h>
#include <iostream>
// Windowing
#include "canvas.h"
#include "renderer/quad.h"
#include "renderer/shader.h"
#include <GLFW/glfw3.h>
#include <string>

const char *TITLE = "LXPAINT";

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS &&
      glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {

    glfwSetWindowShouldClose(window, true);
  }
}
const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

out vec2 TexCoord;

void main() {
  gl_Position = vec4(aPos, 0.0, 1.0);
  TexCoord = aTex;
}
)";
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D canvas;

void main() {
  FragColor = texture(canvas, TexCoord);
}
)";
int main() {
  int windowWidth = 800;
  int windowHeight = 600;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window =
      glfwCreateWindow(windowWidth, windowHeight, TITLE, NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  glViewport(0, 0, windowWidth, windowHeight);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  Canvas canvas(512, 512);
  Shader shader(vertexShaderSource, fragmentShaderSource);
  Quad quad;
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // input → canvas
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    int canvasX = (int)(mx / windowWidth * canvas.width);
    int canvasY = (int)(my / windowHeight * canvas.height);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      canvas.setPixel(canvasX, canvasY, 255, 0, 0, 255);
    }

    canvas.update();

    // render
    shader.use();
    glBindTexture(GL_TEXTURE_2D, canvas.texture);

    quad.draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
