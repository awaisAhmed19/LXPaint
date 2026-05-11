
#pragma once
#include "../Document/Canvas.h"

#include "../Editor/Commands/CommandManager.h"
#include "../Editor/Interaction/ToolInteractionState.h"
#include "../Editor/Preview/PreviewLayer.h"
#include "../Editor/Tools/ToolManager.h"

#include "../Input/InputDispatcher.h"

#include "../Rendering/Renderer.h"

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

public:
  Editor(SDL_Renderer *renderer);
  void handleEvent(const SDL_Event &event);
  void update();
  void render();
  Canvas &getCanvas() { return m_canvas; }
};
