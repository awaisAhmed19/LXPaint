#include <SDL3/SDL.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>

#include "Window.h"

#include "Systems/Logger.h"

namespace App {

Window::Window(const Settings &settings) {
  auto window_flags{static_cast<SDL_WindowFlags>(SDL_WINDOW_HIGH_PIXEL_DENSITY |
                                                 SDL_WINDOW_RESIZABLE)};
  constexpr int window_center_flag{SDL_WINDOWPOS_CENTERED};
  this->m_window = SDL_CreateWindow(settings.title.c_str(), settings.width,
                                    settings.height, window_flags);
  SDL_SetWindowPosition(this->m_window, window_center_flag, window_center_flag);
  this->m_renderer = SDL_CreateRenderer(this->m_window, NULL);

  if (this->m_renderer == nullptr) {
    Logger::err("Error creating SDL_Renderer");
    return;
  }
}
struct WindowSize {
  int width;
  int height;
};

Window::Size Window::size() const {
  Size size{};
  SDL_GetWindowSize(m_window, &size.width, &size.height);
  return size;
}

Window::~Window() {
  SDL_DestroyRenderer(this->m_renderer);
  SDL_DestroyWindow(this->m_window);
}

SDL_Window *Window::nativeWindow() const { return this->m_window; }
SDL_Renderer *Window::nativeRenderer() const { return this->m_renderer; }
}; // namespace App
