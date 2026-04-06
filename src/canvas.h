#include <vector>

struct Canvas {
  int width, height;
  std::vector<unsigned char> pixels;
  GLuint texture;

  Canvas(int w, int h) : width(w), height(h) {
    // RGBA buffer
    pixels.resize(w * h * 4, 255); // white canvas

    // Create texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  void update() {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA,
                    GL_UNSIGNED_BYTE, pixels.data());
  }
  void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b,
                unsigned char a) {
    if (x < 0 || x >= width || y < 0 || y >= height)
      return;

    int idx = (y * width + x) * 4;
    pixels[idx] = r;
    pixels[idx + 1] = g;
    pixels[idx + 2] = b;
    pixels[idx + 3] = a;
  }
};
