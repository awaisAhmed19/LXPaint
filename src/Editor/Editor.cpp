#include "Editor.h"

#include "App/Globals.h"
#include "App/Utils.h"

#include "Document/Canvas.h"

#include "Editor/Interaction/ToolContext.h"

#include "Editor/Tools/AirBrush.h"
#include "Editor/Tools/BaseTool.h"
#include "Editor/Tools/Brush.h"
#include "Editor/Tools/Circle.h"
#include "Editor/Tools/ClickTool.h"
#include "Editor/Tools/CurveLine.h"
#include "Editor/Tools/Eraser.h"
#include "Editor/Tools/EyeDropper.h"
#include "Editor/Tools/FloodFill.h"
#include "Editor/Tools/Lasso.h"
#include "Editor/Tools/Line.h"
#include "Editor/Tools/Magnifier.h"
#include "Editor/Tools/Pencil.h"
#include "Editor/Tools/Rect.h"
#include "Editor/Tools/RectSelection.h"
#include "Editor/Tools/RoundedRect.h"
#include "Editor/Tools/Text.h"
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
constexpr auto AirBrush = "airbrush";
constexpr auto Lasso = "lasso";
constexpr auto RectLasso = "rectlasso";
constexpr auto CurveLine = "curveline";
constexpr auto RoundedRect = "roundedrect";
constexpr auto Magnifier = "magnifier";
constexpr auto Brush = "brush";
constexpr auto Text = "text";
constexpr auto Eyedropper = "eyedropper";
} // namespace ToolID

int cwidth = 800;
int cheight = 500;

Editor::Editor(SDL_Renderer *renderer, const UI::LayoutMetrics &layout)
    : m_canvas(cwidth, cheight), m_preview(cwidth, cheight),
      m_renderer(renderer), m_commands(50, 256), m_docTransform({0.0f, 0.0f}) {
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

  case ToolType::Airbrush:
    m_tools.setActiveTool(ToolID::AirBrush, tool);
    break;

  case ToolType::FreeSelect:
    m_tools.setActiveTool(ToolID::Lasso, tool);
    break;

  case ToolType::RectSelect:
    m_tools.setActiveTool(ToolID::RectLasso, tool);
    break;

  case ToolType::RoundedRectangle:
    m_tools.setActiveTool(ToolID::RoundedRect, tool);
    break;

  case ToolType::Curve:
    m_tools.setActiveTool(ToolID::CurveLine, tool);
    break;

  case ToolType::Text:
    m_tools.setActiveTool(ToolID::Text, tool);
    break;

  case ToolType::Magnifier:
    m_tools.setActiveTool(ToolID::Magnifier, tool);
    break;

  case ToolType::Eyedropper:
    m_tools.setActiveTool(ToolID::Eyedropper, tool);
    break;

  case ToolType::Brush:
    m_tools.setActiveTool(ToolID::Brush, tool);
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
  m_tools.registerTool(ToolID::AirBrush, std::make_unique<AirBrush>());
  m_tools.registerTool(ToolID::Lasso, std::make_unique<Lasso>());
  m_tools.registerTool(ToolID::RectLasso, std::make_unique<RectSelection>());
  m_tools.registerTool(ToolID::CurveLine, std::make_unique<CurveLine>());
  m_tools.registerTool(ToolID::RoundedRect, std::make_unique<RoundedRect>());
  m_tools.registerTool(ToolID::Magnifier, std::make_unique<Magnifier>());
  //  m_tools.registerTool(ToolID::Text, std::make_unique<Text>());
  m_tools.registerTool(ToolID::Eyedropper, std::make_unique<Eyedropper>());
  m_tools.registerTool(ToolID::Brush, std::make_unique<Brush>());
  m_tools.setActiveTool(ToolID::Pencil, ToolType::Pencil);
}
void Editor::setupInputBindings() {
  m_input.keyBinds(SDL_SCANCODE_Z, InputCommand::UNDO);
  m_input.keyBinds(SDL_SCANCODE_Y, InputCommand::REDO);
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
}
void Editor::setFgColor(uint32_t color) { m_fgColor = color; }
void Editor::setBgColor(uint32_t color) { m_bgColor = color; }

ToolContext Editor::makeToolContext() {
  return ToolContext{
      .canvas = &m_canvas,
      .preview = &m_preview,
      .interaction = &m_interaction,
      .commandManager = &m_commands,

      // Snapshot values (for tools that only need to read them)
      .fgColor = m_fgColor,
      .bgColor = m_bgColor,

      //.brushSize = m_toolSettings.brushSize,
      .settings = &m_toolSettings,

      // Live write-back pointers (used by Eyedropper)
      .fgColorOut = &m_fgColor,
      .bgColorOut = &m_bgColor,

      // Used by Magnifier (and later Text if needed)
      .viewport = &m_viewport,
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
  m_viewport.fitCanvasToScreen(); // TODO need to call this if a new document is
                                  // made too
  centerCanvas();

  m_interaction.reset();
  m_tools.reset();

  m_commands.clear();
  Logger::debug(std::format("Canvas resize to {}x{}", w, h));
}

void Editor::setViewportRect(SDL_FRect rect) {
  m_viewport.setScreenRect(rect);

  if (m_viewport.isCanvasLargerThanViewport())
    m_viewport.fitCanvasToScreen();
  else
    m_viewport.clampPan();
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
