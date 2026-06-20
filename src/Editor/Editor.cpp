#include "Editor.h"

#include "App/Globals.h"
#include "App/Utils.h"

#include "Document/Canvas.h"

#include "Editor/Interaction/ToolContext.h"

#include "Editor/Tools/BaseTool.h"
#include "Editor/Tools/Circle.h"
#include "Editor/Tools/ClickTool.h"
#include "Editor/Tools/Eraser.h"
#include "Editor/Tools/FloodFill.h"
#include "Editor/Tools/Line.h"
#include "Editor/Tools/Pencil.h"
#include "Editor/Tools/Rect.h"

#include "Systems/Logger.h"
#include "UI/LayoutEngine/LayoutMetrics.h"
#include "UI/Toolbar.h"
#include <SDL3/SDL_render.h>
namespace ToolID {
constexpr auto Pencil = "pencil";
constexpr auto Line = "line";
constexpr auto Rect = "rect";
constexpr auto Eraser = "eraser";
constexpr auto Circle = "circle";
constexpr auto FloodFill = "floodfill";
} // namespace ToolID

Editor::Editor(SDL_Renderer *renderer, const UI::LayoutMetrics &layout)
    : m_canvas(500, 450), m_preview(500, 450), m_renderer(renderer),
      m_commands(50, 256), m_docTransform({0.0f, 0.0f}) {
  setupTools();
  setupInputBindings();

  m_viewport.setZoom(1.0f);
  m_viewport.setPan({0.0f, 0.0f});
  m_viewport.setScreenRect({layout.viewport.x, layout.viewport.y,
                            layout.viewport.width, layout.viewport.height});

  centerCanvas();
}

void Editor::setActiveTool(ToolType tool) {
  switch (tool) {
  case ToolType::Pencil:
    m_tools.setActiveTool(ToolID::Pencil, tool);
    break;

  case ToolType::Line:
    m_tools.setActiveTool(ToolID::Line, tool);
    break;

  case ToolType::Rectangle:
    m_tools.setActiveTool(ToolID::Rect, tool);
    break;

  case ToolType::Ellipse:
    m_tools.setActiveTool(ToolID::Circle, tool);
    break;

  case ToolType::Eraser:
    m_tools.setActiveTool(ToolID::Eraser, tool);
    break;

  case ToolType::FloodFill:
    m_tools.setActiveTool(ToolID::FloodFill, tool);
    break;

  default:
    break;
  }
}
// move all the tool initalization to the Ui module after it done.... also move
// the input dispatcher too
// Interaction
// ├── InteractionController
// ├── ToolContext
// ├── ToolState
// └── InputDispatcher
//
// move the Rendering pipeline into the
// renderer
// CanvasRenderer
// PreviewRenderer
// viewport [not sure yet]
// Editor
// ├── InteractionController
// ├── Document
// ├── Renderer[rendere,CanvasRenderer,PreviewRenderer]
// ├── History
// └── ToolRegistry
void Editor::setupTools() {
  m_tools.registerTool(ToolID::Pencil, std::make_unique<Pencil>());
  m_tools.registerTool(ToolID::Line, std::make_unique<Line>());
  m_tools.registerTool(ToolID::Rect, std::make_unique<Rect>());
  m_tools.registerTool(ToolID::Eraser, std::make_unique<Eraser>());
  m_tools.registerTool(ToolID::Circle, std::make_unique<Circle>());
  m_tools.registerTool(ToolID::FloodFill, std::make_unique<FloodFill>());
  m_tools.setActiveTool(ToolID::Pencil, ToolType::Pencil);
}

void Editor::setupInputBindings() {
  m_input.keyBinds(SDL_SCANCODE_Z, InputCommand::UNDO);
  m_input.keyBinds(SDL_SCANCODE_Y, InputCommand::REDO);
  m_input.keyBinds(SDL_SCANCODE_P, InputCommand::PENCIL);
  m_input.keyBinds(SDL_SCANCODE_L, InputCommand::LINE);
  m_input.keyBinds(SDL_SCANCODE_R, InputCommand::RECT);
  m_input.keyBinds(SDL_SCANCODE_E, InputCommand::ERASER);
  m_input.keyBinds(SDL_SCANCODE_C, InputCommand::CIRCLE);
  m_input.keyBinds(SDL_SCANCODE_F, InputCommand::FILL);
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
  m_input.bindActions(InputCommand::PENCIL, [this]() {
    m_tools.setActiveTool(ToolID::Pencil, ToolType::Pencil);
  });
  m_input.bindActions(InputCommand::CIRCLE, [this]() {
    m_tools.setActiveTool(ToolID::Circle, ToolType::Ellipse);
  });
  m_input.bindActions(InputCommand::LINE, [this]() {
    m_tools.setActiveTool(ToolID::Line, ToolType::Line);
  });
  m_input.bindActions(InputCommand::RECT, [this]() {
    m_tools.setActiveTool(ToolID::Rect, ToolType::Rectangle);
  });
  m_input.bindActions(InputCommand::ERASER, [this]() {
    m_tools.setActiveTool(ToolID::Eraser, ToolType::Eraser);
  });
  m_input.bindActions(InputCommand::FILL, [this]() {
    m_tools.setActiveTool(ToolID::FloodFill, ToolType::FloodFill);
  });
}

void Editor::setFgColor(uint32_t color) { m_fgColor = color; }
void Editor::setBgColor(uint32_t color) { m_bgColor = color; }

ToolContext Editor::makeToolContext() {
  return ToolContext{
      .canvas = &m_canvas,
      .preview = &m_preview,
      .interaction = &m_interaction,
      .fgColor = m_fgColor,
      .bgColor = m_bgColor,
      .brushSize = m_toolSettings.brushSize,
      .settings = &m_toolSettings,
  };
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
  if (this->m_interaction.active) {
    Logger::warn("Cannot resize: tool in progress");
    return;
  }

  if (w == m_canvas.getWidth() && h == m_canvas.getHeight()) {
    Logger::debug("Resize: dimensions unchanges");
    return;
  }

  if (w <= 0 && h <= 0) {
    Logger::err("Resize: invalid dimensions");
    return;
  }

  m_canvas.resize(w, h, policy);
  m_viewport.onCanvasResized(w, h);
  centerCanvas();

  m_interaction.reset();
  m_tools.reset();

  m_commands.clear();
  Logger::debug(std::format("Canvas resize to {}x{}", w, h));
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
      policy.fill = ResizeFill::BACKGROUNDCOLOR;
      resizeCanvas(m_canvas.getWidth() + 64, m_canvas.getHeight() + 64, policy);
    }

    if (e.key.scancode == SDL_SCANCODE_MINUS && (e.key.mod & SDL_KMOD_CTRL)) {
      ResizePolicy policy;
      policy.anchor = ResizeAnchor::CENTER;
      policy.fill = ResizeFill::BACKGROUNDCOLOR;
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
    BaseTool *tool = m_tools.getActiveTool();
    ClickTool *clicktool = m_tools.getActiveClickTool();
    LX_ASSERT(tool || clicktool, "No active tool");
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

    if (tool) {
      tool->onMouseDown(mousePos, ctx);
    }
    if (clicktool) {
      auto cmd = clicktool->onMouseClick(mousePos, ctx);

      if (cmd)
        m_commands.pushCommand(std::move(cmd), "Flood Fill Executed");

      m_interaction.active = false;
      m_interaction.mouseDown = false;
    }
  }

  /*
    Tool mouse move
  */

  if (m_interaction.active && m_input.leftMouseDown()) {
    BaseTool *tool = m_tools.getActiveTool();
    ClickTool *clicktool = m_tools.getActiveClickTool();
    LX_ASSERT(tool || clicktool, "No active tool");
    if (tool) {
      ToolContext ctx = makeToolContext();

      m_interaction.prevMousePos = m_interaction.currMousePos;
      vec2 mousePos = m_viewport.screenToCanvas(m_input.getMouseScreenPos(),
                                                m_docTransform);
      m_interaction.currMousePos = clampToCanvas(mousePos);

      tool->onMouseMove(m_interaction.currMousePos, ctx);
    }
  }

  /*
    Tool mouse up
  */

  if (m_input.leftMouseReleased()) {
    if (!m_interaction.active)
      return;
    BaseTool *tool = m_tools.getActiveTool();
    ClickTool *clicktool = m_tools.getActiveClickTool();
    LX_ASSERT(tool || clicktool, "No active tool");
    ToolContext ctx = makeToolContext();
    if (tool) {
      vec2 mousePos = m_viewport.screenToCanvas(m_input.getMouseScreenPos(),
                                                m_docTransform);
      mousePos = clampToCanvas(mousePos);
      std::unique_ptr<Command> command = tool->onMouseUp(mousePos, ctx);

      if (command) {
        m_commands.pushCommand(std::move(command), "Draw Stroke");
      }
    }
    m_interaction.active = false;
    m_interaction.mouseDown = false;
  }
}

void Editor::renderUI() {
  /*
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
  ImGui::End();
  */
}

void Editor::centerCanvas() {
  float zoom = m_viewport.getZoom();
  float canvasW = m_canvas.getWidth() * zoom;
  float canvasH = m_canvas.getHeight() * zoom;

  SDL_FRect vp = m_viewport.getScreenRect();

  float x = vp.x + (vp.w - canvasW) * 0.5f;
  float y = vp.y + (vp.h - canvasH) * 0.5f;

  m_viewport.setPan({x, y});
}

void Editor::update() { m_input.beginFrame(); }

void Editor::render() {
  m_renderer.renderTarget(m_canvas, m_viewport, m_docTransform);

  BaseTool *activeTool = m_tools.getActiveTool();

  if (activeTool && activeTool->usesPreview()) {
    m_renderer.renderTarget(m_preview, m_viewport, m_docTransform);
  }
}
