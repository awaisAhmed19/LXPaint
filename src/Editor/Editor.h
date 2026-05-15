
#pragma once
#include "../Document/Canvas.h"
#include "../Editor/Commands/CommandManager.h"
#include "../Editor/Interaction/ToolInteractionState.h"
#include "../Editor/Preview/PreviewLayer.h"
#include "../Editor/Tools/ToolManager.h"
#include "../Input/InputDispatcher.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Transform2D.h"
#include "./Viewport/Viewport.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <SDL3/SDL.h>
class Editor {
private:
  Canvas m_canvas;
  PreviewLayer m_preview;
  Renderer m_renderer;
  CommandManager m_commands;
  ToolManager m_tools;
  ToolInteractionState m_interaction;
  InputDispatcher m_input;
  Viewport m_viewport;
  Transform2D m_docTransform;

public:
  explicit Editor(SDL_Renderer *renderer);
  void handleEvent(const SDL_Event &event);
  void update();
  void render();
  void renderUI();
  vec2 clampToCanvas(vec2 p);
  bool inCanvas(vec2 mousePos);
  Canvas &getCanvas() { return m_canvas; }
};
