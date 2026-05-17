#include "Editor.h"

#include "../Editor/Interaction/ToolContext.h"

#include "../Editor/Tools/Eraser.h"
#include "../Editor/Tools/Line.h"
#include "../Editor/Tools/Pencil.h"
#include "../Editor/Tools/Rect.h"

#include "../Systems/Logger.h"

namespace ToolID {
constexpr auto Pencil = "pencil";
constexpr auto Line = "line";
constexpr auto Rect = "rect";
constexpr auto Eraser = "eraser";
} // namespace ToolID

Editor::Editor(SDL_Renderer *renderer)
    : m_canvas(800, 550), m_preview(800, 550), m_renderer(renderer),
      m_commands(50, 256) {

  setupTools();
  setupInputBindings();

  m_viewport.setZoom(1.0f);
  m_viewport.setPan({0.0f, 0.0f});
  m_viewport.setScreenRect({0, 0, 1280, 720});

  m_docTransform.position = {200, 100};
}

void Editor::setupTools() {
  m_tools.registerTool(ToolID::Pencil, std::make_unique<Pencil>());
  m_tools.registerTool(ToolID::Line, std::make_unique<Line>());
  m_tools.registerTool(ToolID::Rect, std::make_unique<Rect>());
  m_tools.registerTool(ToolID::Eraser, std::make_unique<Eraser>());
  m_tools.setActiveTool(ToolID::Pencil);
}

void Editor::setupInputBindings() {
  m_input.keyBinds(SDL_SCANCODE_Z, InputCommand::UNDO);
  m_input.keyBinds(SDL_SCANCODE_Y, InputCommand::REDO);
  m_input.keyBinds(SDL_SCANCODE_P, InputCommand::PENCIL);
  m_input.keyBinds(SDL_SCANCODE_L, InputCommand::LINE);
  m_input.keyBinds(SDL_SCANCODE_R, InputCommand::RECT);
  m_input.keyBinds(SDL_SCANCODE_E, InputCommand::ERASER);
  m_input.bindActions(InputCommand::UNDO, [this]() {
    if (!m_commands.undo(m_canvas)) {
      Logger::log(LogLevel::DEBUG, "Nothing to undo");
    }
  });
  m_input.bindActions(InputCommand::REDO, [this]() {
    if (!m_commands.redo(m_canvas)) {
      Logger::log(LogLevel::DEBUG, "Nothing to redo");
    }
  });
  m_input.bindActions(InputCommand::PENCIL,
                      [this]() { m_tools.setActiveTool(ToolID::Pencil); });
  m_input.bindActions(InputCommand::LINE,
                      [this]() { m_tools.setActiveTool(ToolID::Line); });
  m_input.bindActions(InputCommand::RECT,
                      [this]() { m_tools.setActiveTool(ToolID::Rect); });
  m_input.bindActions(InputCommand::ERASER,
                      [this]() { m_tools.setActiveTool(ToolID::Eraser); });
}

ToolContext Editor::makeToolContext() {
  return ToolContext{.canvas = &m_canvas,
                     .preview = &m_preview,
                     .interaction = &m_interaction};
}

bool Editor::inCanvas(vec2 mousePos) {
  return mousePos.x >= 0 && mousePos.y >= 0 &&
         mousePos.x < m_canvas.getWidth() && mousePos.y < m_canvas.getHeight();
}

vec2 Editor::clampToCanvas(vec2 p) {
  p.x = std::clamp(p.x, 0.0f, (float)m_canvas.getWidth() - 1);
  p.y = std::clamp(p.y, 0.0f, (float)m_canvas.getHeight() - 1);
  return p;
}

void Editor::handleMouseDown(const SDL_Event &e) {
  if (e.button.button == SDL_BUTTON_MIDDLE) {
    m_panning = true;
    m_lastPanMouse = {(float)e.button.x, (float)e.button.y};
    return;
  }
  if (e.button.button != SDL_BUTTON_LEFT)
    return;
  BaseTool *tool = m_tools.getActive();
  if (!tool)
    return;

  ToolContext ctx = makeToolContext();

  vec2 screenPos = {(float)e.button.x, (float)e.button.y};
  vec2 mousePos = m_viewport.screenToCanvas(screenPos, m_docTransform);

  if (!inCanvas(mousePos))
    return;

  m_interaction.active = true;
  m_interaction.mouseDown = true;

  m_interaction.startMousePos = mousePos;
  m_interaction.currMousePos = mousePos;
  m_interaction.prevMousePos = mousePos;

  tool->onMouseDown(mousePos, ctx);
}

void Editor::handleMouseMove(const SDL_Event &e) {
  if (m_panning) {
    vec2 current = {(float)e.motion.x, (float)e.motion.y};
    vec2 delta = {current.x - m_lastPanMouse.x, current.y - m_lastPanMouse.y};
    m_viewport.setPan(m_viewport.getPan() + delta);
    m_lastPanMouse = current;
    return;
  }
  if (!m_interaction.active)
    return;

  BaseTool *tool = m_tools.getActive();

  if (!tool)
    return;

  ToolContext ctx = makeToolContext();

  m_interaction.prevMousePos = m_interaction.currMousePos;
  vec2 screenPos = {(float)e.motion.x, (float)e.motion.y};
  vec2 mousePos = m_viewport.screenToCanvas(screenPos, m_docTransform);
  m_interaction.currMousePos = clampToCanvas(mousePos);

  tool->onMouseMove(m_interaction.currMousePos, ctx);
}

void Editor::handleMouseUp(const SDL_Event &e) {
  if (e.button.button == SDL_BUTTON_MIDDLE) {
    m_panning = false;
    return;
  }
  if (e.button.button != SDL_BUTTON_LEFT)
    return;

  if (!m_interaction.active)
    return;

  BaseTool *tool = m_tools.getActive();

  if (!tool)
    return;

  ToolContext ctx = makeToolContext();

  vec2 screenPos = {(float)e.button.x, (float)e.button.y};
  vec2 mousePos = m_viewport.screenToCanvas(screenPos, m_docTransform);
  std::unique_ptr<Command> command = tool->onMouseUp(mousePos, ctx);

  if (command) {
    m_commands.pushCommand(std::move(command), "Draw Stroke");
  }

  m_interaction.active = false;
  m_interaction.mouseDown = false;
}

void Editor::handleEvent(const SDL_Event &e) {

  m_input.updateKeyInput(e);

  switch (e.type) {
  case SDL_EVENT_MOUSE_WHEEL: {
    // Zoom event prototype
    float factor = e.wheel.y > 0 ? 1.1f : 0.9f;
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    m_viewport.ZoomAt({(float)mx, (float)my}, factor);
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    handleMouseDown(e);
    break;
  case SDL_EVENT_MOUSE_MOTION:
    handleMouseMove(e);
    break;
  case SDL_EVENT_MOUSE_BUTTON_UP:
    handleMouseUp(e);
    break;
  default:
    break;
  }
}

void Editor::renderUI() {
  ImGui::Begin("Edit History");

  if (ImGui::Button("Undo", ImVec2(100, 0))) {
    m_commands.undo(m_canvas);
  }

  ImGui::SameLine();

  if (ImGui::Button("Redo", ImVec2(100, 0))) {
    m_commands.redo(m_canvas);
  }

  ImGui::Separator();

  ImGui::TextDisabled("%s", m_commands.getDebugInfo().c_str());

  ImGui::End();
}

void Editor::update() {}

void Editor::render() {
  m_renderer.renderTarget(m_canvas, m_viewport, m_docTransform);
  if (m_tools.getActive()->usesPreview()) {
    m_renderer.renderTarget(m_preview, m_viewport, m_docTransform);
  }
}
