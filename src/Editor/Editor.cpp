#include "Editor.h"
#include "../Editor/Interaction/ToolContext.h"
#include "../Editor/Tools/Eraser.h"
#include "../Editor/Tools/Line.h"
#include "../Editor/Tools/Pencil.h"
#include "../Editor/Tools/Rect.h"
#include "../Systems/Logger.h"
Editor::Editor(SDL_Renderer *renderer)
    : m_canvas(1270, 720), m_preview(1270, 720), m_renderer(renderer) {
  this->m_tools.registerTool("pencil", std::make_unique<Pencil>());
  this->m_tools.registerTool("line", std::make_unique<Line>());
  this->m_tools.registerTool("rect", std::make_unique<Rect>());
  this->m_tools.registerTool("eraser", std::make_unique<Eraser>());
  this->m_tools.setActiveTool("pencil");
  // tool = this->m_tools.getActive();
  this->m_input.keyBinds(SDL_SCANCODE_Z, InputCommand::UNDO);
  this->m_input.keyBinds(SDL_SCANCODE_Y, InputCommand::REDO);
  this->m_input.keyBinds(SDL_SCANCODE_P, InputCommand::PENCIL);
  this->m_input.keyBinds(SDL_SCANCODE_F, InputCommand::FILL);
  this->m_input.keyBinds(SDL_SCANCODE_L, InputCommand::LINE);
  this->m_input.keyBinds(SDL_SCANCODE_R, InputCommand::RECT);
  this->m_input.keyBinds(SDL_SCANCODE_E, InputCommand::ERASER);
  this->m_input.bindActions(InputCommand::UNDO, [this]() {
    this->m_commands.undo(this->m_canvas);
    this->m_canvas.markDirty();
  });
  this->m_input.bindActions(InputCommand::REDO, [this]() {
    this->m_commands.redo(this->m_canvas);
    this->m_canvas.markDirty();
  });
  this->m_input.bindActions(InputCommand::PENCIL, [this]() {
    this->m_tools.setActiveTool("pencil");
  });
  this->m_input.bindActions(InputCommand::FILL,
                            [this]() { this->m_tools.setActiveTool("fill"); });
  this->m_input.bindActions(InputCommand::LINE,
                            [this]() { this->m_tools.setActiveTool("line"); });
  this->m_input.bindActions(InputCommand::RECT,
                            [this]() { this->m_tools.setActiveTool("rect"); });
  this->m_input.bindActions(InputCommand::ERASER, [this]() {
    this->m_tools.setActiveTool("eraser");
  });
}

void Editor::handleEvent(const SDL_Event &e) {
  this->m_input.updateKeyInput(e);
  BaseTool *tool = this->m_tools.getActive();

  if (!tool) {
    Logger::log(LogLevel::DEBUG, "TOOL IS NULL MOSTLY LIKELY");
    return;
  }

  ToolContext ctx{.canvas = &this->m_canvas,
                  .preview = &this->m_preview,
                  .interaction = &this->m_interaction};
  switch (e.type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (e.button.button != SDL_BUTTON_LEFT)
      break;
    vec2 mousePos = {static_cast<float>(e.button.x),
                     static_cast<float>(e.button.y)};
    this->m_interaction.active = true;
    this->m_interaction.mouseDown = true;
    this->m_interaction.startMousePos = mousePos;
    this->m_interaction.currMousePos = mousePos;
    this->m_interaction.prevMousePos = mousePos;
    tool->onMouseDown(mousePos, ctx);
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    if (!this->m_interaction.active)
      break;

    this->m_interaction.prevMousePos = this->m_interaction.currMousePos;

    this->m_interaction.currMousePos = {
        static_cast<float>(e.motion.x),
        static_cast<float>(e.motion.y),
    };

    tool->onMouseMove(this->m_interaction.currMousePos, ctx);

    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (e.button.button != SDL_BUTTON_LEFT)
      break;

    if (!this->m_interaction.active)
      break;

    vec2 mousePos = {
        static_cast<float>(e.button.x),
        static_cast<float>(e.button.y),
    };

    std::unique_ptr<Command> command = tool->onMouseUp(mousePos, ctx);

    if (command) {
      this->m_commands.executeCommand(std::move(command), this->m_canvas);
    }

    this->m_interaction.active = false;
    this->m_interaction.mouseDown = false;

    break;
  }
  default:
    break;
  }
}

void Editor::update() {}
void Editor::render() {
  this->m_renderer.begin();
  this->m_renderer.renderTarget(this->m_canvas);
  this->m_renderer.renderTarget(this->m_preview);
  this->m_renderer.end();
}
