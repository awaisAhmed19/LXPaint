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
#include "Editor/Tools/Polygon.h"
#include "Editor/Tools/Rect.h"
#include "Editor/Tools/RectSelection.h"
#include "Editor/Tools/RoundedRect.h"
#include "Editor/Tools/SelectionTool.h"
#include "Editor/Tools/Text.h"
#include "IO/ImageIO.h"
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
constexpr auto Polygon = "polygon";
} // namespace ToolID

int cwidth = 800;
int cheight = 500;

Editor::Editor(SDL_Window *window, SDL_Renderer *renderer,
               const UI::LayoutMetrics &layout)
    : m_window(window), m_document(cwidth, cheight), m_renderer(renderer),
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

  case ToolType::Magnifier:
    m_tools.setActiveTool(ToolID::Magnifier, tool);
    break;

  case ToolType::Eyedropper:
    m_tools.setActiveTool(ToolID::Eyedropper, tool);
    break;

  case ToolType::Brush:
    m_tools.setActiveTool(ToolID::Brush, tool);
    break;

  case ToolType::Text:
    m_tools.setActiveTool(ToolID::Text, tool);
    break;

  case ToolType::Polygon:
    m_tools.setActiveTool(ToolID::Polygon, tool);
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
  m_tools.registerTool(ToolID::Text, std::make_unique<Text>());
  m_tools.registerTool(ToolID::Eyedropper, std::make_unique<Eyedropper>());
  m_tools.registerTool(ToolID::Brush, std::make_unique<Brush>());
  m_tools.registerTool(ToolID::Polygon, std::make_unique<Polygon>());
  m_tools.setActiveTool(ToolID::Pencil, ToolType::Pencil);
}
void Editor::setupInputBindings() {
  m_input.keyBinds(SDL_SCANCODE_Z, InputCommand::UNDO);
  m_input.keyBinds(SDL_SCANCODE_Y, InputCommand::REDO);
  m_input.keyBinds(SDL_SCANCODE_S, InputCommand::SAVE);
  m_input.keyBinds(SDL_SCANCODE_O, InputCommand::OPEN);
  m_input.keyBinds(SDL_SCANCODE_N, InputCommand::NEW_DOCUMENT);
  m_input.bindActions(InputCommand::UNDO, [this]() {
    if (!m_commands.undo(m_document.getCanvas())) {
      Logger::log(LogLevel::DEBUG, "Nothing to undo");
    }
  });
  m_input.bindActions(InputCommand::REDO, [this]() {
    if (!m_commands.redo(m_document.getCanvas())) {
      Logger::log(LogLevel::DEBUG, "Nothing to redo");
    }
  });
  m_input.bindActions(InputCommand::SAVE, [this]() { saveDocument(); });
  m_input.bindActions(InputCommand::SAVE_AS, [this]() { saveDocumentAs(); });
  m_input.bindActions(InputCommand::OPEN, [this]() { openDocument(); });
  m_input.bindActions(InputCommand::NEW_DOCUMENT, [this]() { newDocument(); });
}
void Editor::setFgColor(uint32_t color) { m_fgColor = color; }
void Editor::setBgColor(uint32_t color) { m_bgColor = color; }

ToolContext Editor::makeToolContext() {
  return ToolContext{
      .canvas = &m_document.getCanvas(),
      .preview = &m_document.getPreview(),
      .interaction = &m_interaction,
      .commandManager = &m_commands,
      .fgColor = m_fgColor,
      .bgColor = m_bgColor,
      .settings = &m_toolSettings,
      .fgColorOut = &m_fgColor,
      .bgColorOut = &m_bgColor,
      .colorSampledOut = &m_colorSampledThisFrame,
      .viewport = &m_viewport,
      .window = m_window,

  };
}

bool Editor::inCanvas(vec2 mousePos) {
  return mousePos.x >= 0 && mousePos.y >= 0 &&
         mousePos.x < m_document.getCanvas().getWidth() &&
         mousePos.y < m_document.getCanvas().getHeight();
}

vec2 Editor::clampToCanvas(vec2 p) {
  p.x = std::clamp(p.x, 0.0f, (float)m_document.getCanvas().getWidth() - 1);
  p.y = std::clamp(p.y, 0.0f, (float)m_document.getCanvas().getHeight() - 1);
  return p;
}

void Editor::resizeCanvas(int w, int h, const ResizePolicy &policy) {
  if (this->m_interaction.active) {
    Logger::warn("Cannot resize: tool in progress");
    return;
  }

  if (w == m_document.getCanvas().getWidth() &&
      h == m_document.getCanvas().getHeight()) {
    Logger::debug("Resize: dimensions unchanges");
    return;
  }

  if (w <= 0 && h <= 0) {
    Logger::err("Resize: invalid dimensions");
    return;
  }

  m_document.resize(w, h, policy);
  // m_document.getPreview().resize(w, h, policy);
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

  // Route key-down to the active tool BEFORE the existing Ctrl+=/Ctrl+-
  // resize shortcuts, so a tool that wants to consume a key (Polygon's
  // Escape, Text's full editing keyset) gets first refusal. If the tool
  // consumes it (returns true), the global resize shortcut below is
  // skipped for this event.
  bool keyConsumedByTool = false;

  if (e.type == SDL_EVENT_KEY_DOWN) {
    BaseTool *activeTool = m_tools.getActiveTool();
    if (activeTool) {
      ToolContext ctx = makeToolContext();
      keyConsumedByTool = activeTool->onKeyDown(e.key.scancode, ctx);

      // Text specifically may have produced a commit as a side effect of
      // this key (Ctrl+Enter) — see Text::onKeyDown / m_pendingCommit.
      // Editor is the only place that can reach into the concrete Text
      // type to retrieve it; every other tool only ever produces a
      // Command via onMouseUp's return value, which is handled separately
      // further down in this function exactly as before.
      if (auto *textTool = dynamic_cast<Text *>(activeTool)) {
        if (auto cmd = textTool->takePendingCommit()) {
          m_commands.pushCommand(std::move(cmd), "Text");
          m_document.markModified();
        }
      }
    }
  }

  if (e.type == SDL_EVENT_TEXT_INPUT) {
    BaseTool *activeTool = m_tools.getActiveTool();
    if (activeTool) {
      ToolContext ctx = makeToolContext();
      keyConsumedByTool =
          activeTool->onTextInput(e.text.text, ctx) || keyConsumedByTool;
    }
  }

  // temp resize
  //
  // BLOCK
  if (e.type == SDL_EVENT_KEY_DOWN && !keyConsumedByTool) {
    if (e.key.scancode == SDL_SCANCODE_EQUALS && (e.key.mod & SDL_KMOD_CTRL)) {
      ResizePolicy policy;
      policy.anchor = ResizeAnchor::CENTER;
      policy.fill = ResizeFill::BACKGROUNDCOLOR;
      resizeCanvas(m_document.getCanvas().getWidth() + 64,
                   m_document.getCanvas().getHeight() + 64, policy);
    }

    if (e.key.scancode == SDL_SCANCODE_MINUS && (e.key.mod & SDL_KMOD_CTRL)) {
      ResizePolicy policy;
      policy.anchor = ResizeAnchor::CENTER;
      policy.fill = ResizeFill::BACKGROUNDCOLOR;
      resizeCanvas(std::max(64, m_document.getCanvas().getWidth() - 64),
                   std::max(64, m_document.getCanvas().getHeight() - 64),
                   policy);
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

      if (cmd) {
        m_commands.pushCommand(std::move(cmd), "Clicktool Executed");
        m_document.markModified();
      }

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
  } else if (!m_input.leftMouseDown()) {
    // Hover-only move: no button held. Unlike the block above, this does
    // NOT require m_interaction.active — Polygon is "drawing" across many
    // independent clicks with the mouse up in between, not a single
    // press-drag-release like Pencil/Line/Rect, so it needs move events
    // even while m_interaction.active is false between vertex placements.
    // Only tools that opt in via wantsHoverMoves() get this; every
    // existing tool defaults to false (see BaseTool.h) and is unaffected.
    BaseTool *tool = m_tools.getActiveTool();
    if (tool && tool->wantsHoverMoves()) {
      ToolContext ctx = makeToolContext();
      vec2 mousePos = m_viewport.screenToCanvas(m_input.getMouseScreenPos(),
                                                m_docTransform);
      mousePos = clampToCanvas(mousePos);
      tool->onMouseMove(mousePos, ctx);
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
        m_document.markModified();
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
    m_commands.undo(m_document.canvas());
  }
  ImGui::SameLine();
  if (ImGui::Button("Redo", ImVec2(100, 0))) {
    m_commands.redo(m_document.canvas());
  }
  ImGui::Separator();
  ImGui::TextDisabled("%s", m_commands.getDebugInfo().c_str());
  ImGui::End();
  */
}
void Editor::newDocument() {
  if (m_document.isModified()) {
    Logger::warn("TODO: Prompt to save changes.");
    // return if user cancels
  }

  ResizePolicy policy;
  policy.anchor = ResizeAnchor::TOPLEFT;
  policy.fill = ResizeFill::BACKGROUNDCOLOR;

  m_document.resize(800, 500, policy);
  m_document.clear(COLORS::WHITE);

  m_document.setPath({});
  m_document.markSaved();

  m_commands.clear();
  m_tools.reset();
  m_interaction.reset();

  centerCanvas();
}

bool Editor::saveDocumentAs() {
  std::filesystem::path path = "test.png";

  if (!ImageIO::save(m_document.getCanvas(), path))
    return false;

  m_document.setPath(path);
  m_document.markSaved();

  return true;
}

bool Editor::saveDocument() {
  if (!m_document.hasPath())
    return saveDocumentAs();

  if (!ImageIO::save(m_document.getCanvas(), m_document.path()))
    return false;

  m_document.markSaved();

  return true;
}

bool Editor::openDocument() {
  std::filesystem::path path = "test.png";
  // when dialog box is implemented std::filesystem::path path =
  // FileDialog::save();
  if (!ImageIO::load(m_document.getCanvas(), path))
    return false;

  m_document.setPath(path);
  m_document.markSaved();

  m_commands.clear();
  centerCanvas();

  return true;
}

void Editor::centerCanvas() {
  float zoom = m_viewport.getZoom();
  float canvasW = m_document.getCanvas().getWidth() * zoom;
  float canvasH = m_document.getCanvas().getHeight() * zoom;

  SDL_FRect vp = m_viewport.getScreenRect();

  float x = vp.x + (vp.w - canvasW) * 0.5f;
  float y = vp.y + (vp.h - canvasH) * 0.5f;

  m_viewport.setPan({x, y});
}

void Editor::invertColors() {
  if (m_interaction.active) {
    Logger::warn("Cannot invert colors: tool in progress");
    return;
  }

  Canvas &canvas = m_document.getCanvas();
  SDL_Rect bounds{0, 0, canvas.getWidth(), canvas.getHeight()};

  auto cmd = std::make_unique<SnapshotCommand>(canvas.getSurface(), bounds);
  m_document.invertColors();
  cmd->captureAfter(canvas.getSurface());

  m_commands.pushCommand(std::move(cmd), "Invert Colors");
  m_document.markModified();
}

void Editor::flipHorizontal() {
  if (m_interaction.active) {
    Logger::warn("Cannot flip: tool in progress");
    return;
  }

  Canvas &canvas = m_document.getCanvas();
  SDL_Rect bounds{0, 0, canvas.getWidth(), canvas.getHeight()};

  auto cmd = std::make_unique<SnapshotCommand>(canvas.getSurface(), bounds);
  m_document.flipHorizontal();
  cmd->captureAfter(canvas.getSurface());

  m_commands.pushCommand(std::move(cmd), "Flip Horizontal");
  m_document.markModified();
}

void Editor::flipVertical() {
  if (m_interaction.active) {
    Logger::warn("Cannot flip: tool in progress");
    return;
  }

  Canvas &canvas = m_document.getCanvas();
  SDL_Rect bounds{0, 0, canvas.getWidth(), canvas.getHeight()};

  auto cmd = std::make_unique<SnapshotCommand>(canvas.getSurface(), bounds);
  m_document.flipVertical();
  cmd->captureAfter(canvas.getSurface());

  m_commands.pushCommand(std::move(cmd), "Flip Vertical");
  m_document.markModified();
}

void Editor::rotate90CW() {
  if (m_interaction.active) {
    Logger::warn("Cannot rotate: tool in progress");
    return;
  }

  // Rotation changes canvas dimensions, which SnapshotCommand's
  // fixed-bounds before/after model cannot represent (it assumes the
  // canvas rect is the same shape before and after — true for invert/
  // flip/clear, false here). Rather than silently producing a corrupt
  // undo entry, rotation clears history the same way resizeCanvas()
  // already does for dimension changes.
  m_document.rotate90CW();
  m_viewport.onCanvasResized(m_document.getCanvas().getWidth(),
                             m_document.getCanvas().getHeight());
  m_viewport.fitCanvasToScreen();
  centerCanvas();
  m_commands.clear();

  Logger::debug("Rotated canvas 90° CW");
}

void Editor::rotate90CCW() {
  if (m_interaction.active) {
    Logger::warn("Cannot rotate: tool in progress");
    return;
  }

  m_document.rotate90CCW();
  m_viewport.onCanvasResized(m_document.getCanvas().getWidth(),
                             m_document.getCanvas().getHeight());
  m_viewport.fitCanvasToScreen();
  centerCanvas();
  m_commands.clear();

  Logger::debug("Rotated canvas 90° CCW");
}

void Editor::clearImage() {
  if (m_interaction.active) {
    Logger::warn("Cannot clear: tool in progress");
    return;
  }

  Canvas &canvas = m_document.getCanvas();
  SDL_Rect bounds{0, 0, canvas.getWidth(), canvas.getHeight()};

  auto cmd = std::make_unique<SnapshotCommand>(canvas.getSurface(), bounds);
  m_document.clear(m_bgColor);
  cmd->captureAfter(canvas.getSurface());

  m_commands.pushCommand(std::move(cmd), "Clear Image");
  m_document.markModified();
}

void Editor::selectAll() {
  BaseTool *tool = m_tools.getActiveTool();
  if (auto *sel = dynamic_cast<SelectionTool *>(tool)) {
    ToolContext ctx = makeToolContext();
    sel->selectAllCanvas(ctx);
  } else {
    Logger::warn("Editor::selectAll — no active selection tool");
  }
}

void Editor::clearSelection() {
  BaseTool *tool = m_tools.getActiveTool();
  if (auto *sel = dynamic_cast<SelectionTool *>(tool)) {
    sel->clearSelection();
  } else {
    Logger::warn("Editor::clearSelection — no active selection tool");
  }
}

void Editor::setFullscreen(bool fullscreen) {
  if (!m_window)
    return;

  SDL_SetWindowFullscreen(m_window, fullscreen);
  m_fullscreen = fullscreen;
  Logger::debug(std::format("Fullscreen: {}", fullscreen));
}
void Editor::update() { m_input.beginFrame(); }

void Editor::render() {
  m_renderer.renderTarget(m_document.getCanvas(), m_viewport, m_docTransform);

  BaseTool *activeTool = m_tools.getActiveTool();

  if (activeTool && activeTool->usesPreview()) {
    m_renderer.renderTarget(m_document.getPreview(), m_viewport,
                            m_docTransform);
  }
}
