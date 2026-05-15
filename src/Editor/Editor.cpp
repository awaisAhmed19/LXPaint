#include "Editor.h"
#include "../Editor/Interaction/ToolContext.h"
#include "../Editor/Tools/Eraser.h"
#include "../Editor/Tools/Line.h"
#include "../Editor/Tools/Pencil.h"
#include "../Editor/Tools/Rect.h"
#include "../Systems/Logger.h"
Editor::Editor(SDL_Renderer *renderer)
    : m_canvas(800, 550), m_preview(800, 550), m_renderer(renderer),
      m_commands(50, 256) {
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
    if (this->m_commands.undo(this->m_canvas)) {

      this->m_canvas.markDirty();
    } else {
      Logger::log(LogLevel::DEBUG, "Nothing to undo");
    }
  });
  this->m_input.bindActions(InputCommand::REDO, [this]() {
    if (this->m_commands.redo(this->m_canvas)) {
      this->m_canvas.markDirty();
    } else {
      Logger::log(LogLevel::DEBUG, "Nothing to redo");
    }
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
  this->m_viewport.setZoom(1.0f);
  this->m_viewport.setPan({0.0, 0.0});
  this->m_viewport.setScreenRect({0, 0, 1280, 720});

  this->m_docTransform.position = {200, 100};
}

void Editor::renderUI() {
  ImGui::Begin("Edit History");

  // Undo button
  if (ImGui::Button("Undo", ImVec2(100, 0))) {
    m_commands.undo(m_canvas);
    m_canvas.markDirty();
  }
  ImGui::SameLine();
  if (auto desc = m_commands.getUndoDescription()) {
    ImGui::Text("(%s)", desc->c_str());
  } else {
    ImGui::TextDisabled("(Nothing to undo)");
  }

  ImGui::SameLine();
  if (ImGui::Button("Redo", ImVec2(100, 0))) {
    m_commands.redo(m_canvas);
    m_canvas.markDirty();
  }
  ImGui::SameLine();
  if (auto desc = m_commands.getRedoDescription()) {
    ImGui::Text("(%s)", desc->c_str());
  } else {
    ImGui::TextDisabled("(Nothing to redo)");
  }

  // Memory usage bar
  float memPercent = m_commands.getMemoryUsagePercent();
  ImGui::ProgressBar(memPercent / 100.0f, ImVec2(-1, 0));
  ImGui::SameLine();
  ImGui::Text("%.1f%%", memPercent);

  ImGui::TextDisabled("%s", m_commands.getDebugInfo().c_str());

  ImGui::End();
}

bool Editor::inCanvas(vec2 mousePos) {
  return mousePos.x >= 0 && mousePos.y >= 0 &&
         mousePos.x < this->m_canvas.getWidth() &&
         mousePos.y < this->m_canvas.getHeight();
}
vec2 Editor::clampToCanvas(vec2 p) {

  p.x = std::clamp(p.x, 0.0f, (float)m_canvas.getWidth() - 1);

  p.y = std::clamp(p.y, 0.0f, (float)m_canvas.getHeight() - 1);

  return p;
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
    vec2 screenPos = {(float)e.button.x, (float)e.button.y};

    vec2 worldPos = this->m_viewport.screenToWorld(screenPos);
    vec2 mousePos = {worldPos.x - this->m_docTransform.position.x,

                     worldPos.y - this->m_docTransform.position.y};
    if (!inCanvas(mousePos))
      break;
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

    vec2 screenPos = {
        (float)(e.motion.x),
        (float)(e.motion.y),
    };

    vec2 worldPos = this->m_viewport.screenToWorld(screenPos);

    this->m_interaction.currMousePos =
        clampToCanvas({worldPos.x - this->m_docTransform.position.x,

                       worldPos.y - this->m_docTransform.position.y});
    // if (!inCanvas(this->m_interaction.currMousePos))
    //   break;
    tool->onMouseMove(this->m_interaction.currMousePos, ctx);

    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (e.button.button != SDL_BUTTON_LEFT)
      break;

    if (!this->m_interaction.active)
      break;

    vec2 screenPos = {
        (float)(e.motion.x),
        (float)(e.motion.y),
    };

    vec2 worldPos = this->m_viewport.screenToWorld(screenPos);

    vec2 mousePos = {worldPos.x - this->m_docTransform.position.x,

                     worldPos.y - this->m_docTransform.position.y};
    std::unique_ptr<Command> command = tool->onMouseUp(mousePos, ctx);

    if (command) {
      this->m_commands.executeCommand(std::move(command), this->m_canvas,
                                      "Draw Stroke");
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
  this->m_renderer.renderTarget(this->m_canvas, this->m_viewport,
                                this->m_docTransform);
  this->m_renderer.renderTarget(this->m_preview, this->m_viewport,
                                this->m_docTransform);
}
