
#pragma once
#include "../Document/Canvas.h"
#include "../Editor/Commands/CommandManager.h"
#include "../Editor/Interaction/ToolInteractionState.h"
#include "../Editor/Preview/PreviewLayer.h"
#include "../Editor/Tools/ToolManager.h"
#include "../Input/InputDispatcher.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Transform2D.h"
#include "./Interaction/ToolContext.h"
#include "./Viewport/Viewport.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
class Editor {
private:
  // bool m_panning = false;
  // vec2 m_lastPanMouse{0.0f, 0.0f};
  // bool m_spaceHeld = false;

  Canvas m_canvas;
  PreviewLayer m_preview;
  Renderer m_renderer;
  CommandManager m_commands;
  ToolManager m_tools;
  ToolInteractionState m_interaction;
  InputDispatcher m_input;
  Viewport m_viewport;
  Transform2D m_docTransform;
  void setupTools();
  void setupInputBindings();

  ToolContext makeToolContext();

  void handleMouseDown(const SDL_Event &e);
  void handleMouseMove(const SDL_Event &e);
  void handleMouseUp(const SDL_Event &e);
  // vec2 screenToCanvas(vec2 screenPos) const;

public:
  explicit Editor(SDL_Renderer *renderer);
  void handleEvent(const SDL_Event &event);
  void update();
  void centerCanvas();
  void render();
  void renderUI();
  vec2 clampToCanvas(vec2 p);
  bool inCanvas(vec2 mousePos);
  Canvas &getCanvas() { return m_canvas; }
};
