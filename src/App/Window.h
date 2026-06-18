#pragma once

#include <SDL3/SDL.h>

#include <string>

namespace App {

class Window {
public:
  struct Settings {
    std::string title;
    int width = 1280;
    int height = 720;
  };

  struct Size {
    int width;
    int height;
  };

  explicit Window(const Settings &settings);
  ~Window();

  [[nodiscard]]
  Size size() const;

  [[nodiscard]]
  SDL_Window *nativeWindow() const;

  [[nodiscard]]
  SDL_Renderer *nativeRenderer() const;

private:
  SDL_Window *m_window = nullptr;
  SDL_Renderer *m_renderer = nullptr;
};

} // namespace App
