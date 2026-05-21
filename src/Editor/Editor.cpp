#include "Editor.h"

#include "Editor/Interaction/ToolContext.h"

#include "Editor/Tools/Eraser.h"
#include "Editor/Tools/Line.h"
#include "Editor/Tools/Pencil.h"
#include "Editor/Tools/Rect.h"

#include "Systems/Logger.h"

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

  centerCanvas();
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
void Editor::resizeCanvas(int w, int h, const ResizePolicy &policy) {
  m_canvas.resize(w, h, policy);
  m_preview.allocate(w, h);
  centerCanvas();
  Logger::debug(std::format("Canvas resized to {}x{}", w, h));
}
void Editor::handleEvent(const SDL_Event &e) {

  ImGuiIO &io = ImGui::GetIO();

  // m_input.beginFrame();
  m_input.update(e);
  // temp resize
  //
  // BLOCK
  if (e.type == SDL_EVENT_KEY_DOWN) {

    if (e.key.scancode == SDL_SCANCODE_EQUALS && (e.key.mod & SDL_KMOD_CTRL)) {

      ResizePolicy policy;
      policy.anchor = ResizeAnchor::CENTER;

      resizeCanvas(m_canvas.getWidth() + 64, m_canvas.getHeight() + 64, policy);
    }

    if (e.key.scancode == SDL_SCANCODE_MINUS && (e.key.mod & SDL_KMOD_CTRL)) {

      ResizePolicy policy;
      policy.anchor = ResizeAnchor::CENTER;

      resizeCanvas(std::max(64, m_canvas.getWidth() - 64),
                   std::max(64, m_canvas.getHeight() - 64), policy);
    }
  }
  // DEBUG BLOCK ENDs
  /*
    UI owns input
  */

  if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
    return;
  }

  /*
    Zoom
  */

  if (m_input.zoomTriggered()) {
    m_viewport.ZoomAt(m_input.getMouseScreenPos(), m_input.getZoomFactor());
  }

  /*
    Pan
  */

  if (m_input.isPanning()) {
    m_viewport.setPan(m_viewport.getPan() + m_input.getMouseDelta());
    return;
  }

  /*
    Tool mouse down
  */

  if (m_input.leftMousePressed()) {

    BaseTool *tool = m_tools.getActive();

    LX_ASSERT(tool != nullptr, "Active tool missing");

    ToolContext ctx = makeToolContext();

    vec2 mousePos =
        m_viewport.screenToCanvas(m_input.getMouseScreenPos(), m_docTransform);

    if (!inCanvas(mousePos))
      return;

    mousePos = clampToCanvas(mousePos);

    m_interaction.active = true;
    m_interaction.mouseDown = true;

    m_interaction.startMousePos = mousePos;
    m_interaction.currMousePos = mousePos;
    m_interaction.prevMousePos = mousePos;

    tool->onMouseDown(mousePos, ctx);
  }

  /*
    Tool mouse move
  */

  if (m_interaction.active && m_input.leftMouseDown()) {

    BaseTool *tool = m_tools.getActive();

    LX_ASSERT(tool != nullptr, "Active tool missing");
    ToolContext ctx = makeToolContext();
    m_interaction.prevMousePos = m_interaction.currMousePos;
    vec2 mousePos =
        m_viewport.screenToCanvas(m_input.getMouseScreenPos(), m_docTransform);
    m_interaction.currMousePos = clampToCanvas(mousePos);
    tool->onMouseMove(m_interaction.currMousePos, ctx);
  }

  /*
    Tool mouse up
  */

  if (m_input.leftMouseReleased()) {

    if (!m_interaction.active)
      return;

    BaseTool *tool = m_tools.getActive();
    LX_ASSERT(tool != nullptr, "Active tool missing");
    ToolContext ctx = makeToolContext();

    vec2 mousePos =
        m_viewport.screenToCanvas(m_input.getMouseScreenPos(), m_docTransform);

    std::unique_ptr<Command> command = tool->onMouseUp(mousePos, ctx);

    if (command) {
      m_commands.pushCommand(std::move(command), "Draw Stroke");
    }

    m_interaction.active = false;
    m_interaction.mouseDown = false;
  }
}

void Editor::renderUI() {

  ImGui::Begin("History");

  if (ImGui::Button("Undo", ImVec2(100, 0))) {
    m_commands.undo(m_canvas);
  }

  ImGui::SameLine();
  if (ImGui::Button("Redo", ImVec2(100, 0))) {
    m_commands.redo(m_canvas);
  }

  ImGui::Separator();
  ImGui::TextDisabled("%s", m_commands.getDebugInfo().c_str());

  /*
    Optional quick stats
  */

  ImGui::Separator();

  ImGui::Text("Undo Count: %d", (int)m_commands.undoCount());
  ImGui::Text("Redo Count: %d", (int)m_commands.redoCount());
  ImGui::Text("Memory Usage: %.2f MB", m_commands.memoryUsageMB());

  ImGui::End();
}

void Editor::centerCanvas() {

  int w = 0;
  int h = 0;

  SDL_GetRenderOutputSize(m_renderer.getSDLRenderer(), &w, &h);
  float zoom = m_viewport.getZoom();
  float canvasW = m_canvas.getWidth() * zoom;
  float canvasH = m_canvas.getHeight() * zoom;
  float x = (w - canvasW) * 0.5f;
  float y = (h - canvasH) * 0.5f;

  m_viewport.setPan({x, y});
}

void Editor::update() { m_input.beginFrame(); }

void Editor::render() {
  m_renderer.renderTarget(m_canvas, m_viewport, m_docTransform);
  if (m_tools.getActive()->usesPreview()) {
    m_renderer.renderTarget(m_preview, m_viewport, m_docTransform);
  }
}
