#pragma once

#include <SDL3/SDL.h>

#include <string>

namespace App {
class Window {
public:
  struct Settings {
    std::string title;
    const int width{1280};
    const int height{720};
  };

  explicit Window(const Settings &settings);
  ~Window();

  [[nodiscard]] SDL_Window *getNativeWindow() const;
  [[nodiscard]] SDL_Renderer *getNativeRenderer() const;

private:
  SDL_Window *m_window{nullptr};
  SDL_Renderer *m_renderer{nullptr};
};
} // namespace App
