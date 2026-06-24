#pragma once

#include "BaseTool.h"
#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/ToolSettings.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>
/*
  Text — Microsoft-Paint-style text tool.

  Requires SDL_ttf (TTF_Font / TTF_RenderText_Blended). Nothing else in this
  codebase rasterizes glyphs onto an SDL_Surface, so this is a new build
  dependency: link SDL3_ttf.

  Font lifecycle (path, TTF_Init/TTF_Quit, open/close) is owned by
  App::Application, not by Text:
    - App::Application::Application() calls TTF_Init(), then
      Text::initFontSystem(fontPath) once, after SDL_Init succeeds.
    - App::Application::~Application() calls Text::shutdownFontSystem(),
      then TTF_Quit(), before SDL_Quit().
  Text itself never opens a font path or calls TTF_Init/TTF_Quit — it only
  reads the already-opened TTF_Font* through s_font. This keeps the actual
  asset path in exactly one place (Application), so swapping a hardcoded
  relative path for a real asset-resolution call later is a one-line change
  at the initFontSystem call site, not a change to Text.

  State machine:
    Idle    -> onMouseDown starts a drag (like Rect/Circle)
    Placing -> onMouseMove resizes the rect; onMouseUp with a non-degenerate
               rect enters Editing (SDL_StartTextInput is turned on here)
    Editing -> onKeyDown / onTextInput edit the buffer; clicking inside the
               rect moves the caret; clicking outside the rect, or
               Ctrl+Enter, commits; Escape cancels.

  Buffer model: a single std::string with embedded '\n' for explicit
  (user-pressed-Enter) line breaks, plus a byte-index caret. Word-wrap is
  display-only — recomputed from the buffer + rect width every time it's
  needed, never written back into the buffer — so wrapped breaks never get
  confused with real newlines (this matters for Home/End, Up/Down, and
  clipboard round-tripping).
*/

class Text : public BaseTool {
public:
  struct Style {
    // Font family/path selection is explicitly a stub per spec ("only the
    // default font must be functional") and, separately, font *loading* is
    // no longer Text's responsibility at all — see initFontSystem above.
    // A per-Style path field would imply Text can open arbitrary fonts
    // per-activation, which it deliberately cannot; left out rather than
    // kept as a misleading unused stub.
    int pointSize = 16;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikeout = false;
  };

private:
  enum class State { Idle, Placing, Editing };

  // A single visual (wrapped) line: byte-range [start, end) into m_buffer,
  // plus its baseline-independent top-left draw position relative to the
  // text rect's top-left.
  struct WrapLine {
    size_t start;
    size_t end; // exclusive; does not include the line's own trailing '\n'
                // if any — '\n' bytes are consumed by the wrap pass but not
                // included in any WrapLine's [start,end) range.
    int y;      // pixel offset from rect top
  };

  State m_state = State::Idle;

  SDL_Rect m_rect{0, 0, 0, 0}; // editing rect in canvas space
  vec2 m_dragStart{0.f, 0.f};

  std::string m_buffer;
  size_t m_caret = 0; // byte index into m_buffer, always at a UTF-8
                      // boundary (only ASCII nav is implemented below;
                      // multi-byte UTF-8 caret stepping is noted as a
                      // follow-up — see onTextInput / moveCaret comments)

  // Mouse text selection is explicitly out of scope per spec, but Ctrl+A/
  // C/X are required. Without a selection-range model, the only coherent
  // meaning for "select all" is a whole-buffer flag that Ctrl+C/X consult:
  // Ctrl+A arms it (no visual highlight is drawn — nothing in the spec
  // requires one), any other key disarms it, and Ctrl+C/X act on the
  // entire buffer only while it's armed (otherwise they're no-ops, since
  // there is no selection to copy/cut).
  bool m_selectAll = false;

  Style m_style;
  ToolSettings::BackgroundMode m_bgMode = ToolSettings::BackgroundMode::Opaque;

  // BaseTool::deactivate() takes no ToolContext (see BaseTool.h — this is
  // true for every tool, not something Text introduced), so it has no
  // direct way to reach ctx.window for the SDL_StopTextInput call it needs
  // to make if the tool is switched away from mid-edit. Cached here the
  // first time a ToolContext is actually seen (enterEditing) so deactivate
  // can still call SDL_StopTextInput correctly without a signature change
  // that would ripple through every other tool.
  SDL_Window *m_window = nullptr;

  std::vector<WrapLine> m_wrapCache;
  bool m_wrapDirty = true;

  std::unique_ptr<SnapshotCommand> m_command;

  // onMouseDown/onKeyDown cannot themselves return a Command (their
  // signatures are void/bool, matching every other BaseTool), but two
  // input paths legitimately need to commit immediately: clicking outside
  // the rect while editing, and Ctrl+Enter. Both stash the result here;
  // Editor::handleEvent retrieves it via takePendingCommit() right after
  // dispatching the mouse-up or key-down event that produced it — see the
  // integration notes for the exact call sites.
  std::unique_ptr<Command> m_pendingCommit;
  bool m_commitArmedFromOutsideClick = false;

  // Font lifecycle is owned by the application, not by Text itself —
  // see initFontSystem/shutdownFontSystem below. This replaces an earlier
  // version where Text lazily opened a hardcoded "../fonts/default.ttf"
  // path on first use and never freed it: that broke under any installed/
  // portable/packaged build layout, and there was no code anywhere that
  // could correctly call TTF_CloseFont since no owner outside Text held
  // the pointer. Application now owns the path and the font handle.
  static TTF_Font *s_font;
  static bool ensureFont(int pointSize);

  void rebuildWrapIfNeeded();
  void wrapBuffer(int wrapWidth);

  // Caret <-> (wrapLine index, column-in-bytes) mapping, needed because
  // Up/Down must move by *visual* line, not by '\n'-delimited line.
  size_t wrapLineForCaret() const;
  void moveCaretVertical(int dLines);
  void moveCaretHome();
  void moveCaretEnd();

  void insertText(const char *utf8);
  void deleteBackward();
  void deleteForward();

  int lineHeight() const;
  int textWidth(const char *utf8Start, size_t byteLen) const;

  void redrawPreview(ToolContext &ctx);

  bool pointInRect(vec2 pos) const;
  void enterEditing(ToolContext &ctx);
  std::unique_ptr<Command> commit(ToolContext &ctx);
  void cancel(ToolContext &ctx);

public:
  ~Text() override;

  // Application-owned font lifecycle. App::Application calls
  // initFontSystem() once at startup (after TTF_Init succeeds) with
  // whatever path it resolves the default font to, and
  // shutdownFontSystem() once at shutdown (before TTF_Quit). No Text
  // instance calls TTF_OpenFont/TTF_CloseFont itself — every instance just
  // reads the shared s_font that Application set up. Returns false if the
  // font failed to load; Text remains constructible and usable (it simply
  // can't render glyphs — ensureFont's caller already logs and no-ops in
  // that case, same as before).
  static bool initFontSystem(const std::string &fontPath,
                             int defaultPointSize = 16);
  static void shutdownFontSystem();

  bool usesPreview() const override { return true; }

  // Editing needs hover-independent move (caret blink position tracking is
  // not required, but click-to-move-caret while no drag is happening still
  // routes through onMouseDown only — no hover moves are actually required
  // for Text, unlike Polygon. Kept false; listed for symmetry/clarity.)
  bool wantsHoverMoves() const override { return false; }

  bool isEditing() const { return m_state == State::Editing; }

  // See m_pendingCommit comment above — Editor calls this after dispatching
  // a mouse-up or key-down to Text, and pushes the result to CommandManager
  // exactly like any other tool's onMouseUp return value.
  std::unique_ptr<Command> takePendingCommit() {
    return std::move(m_pendingCommit);
  }

  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;

  bool onKeyDown(SDL_Scancode scancode, ToolContext &ctx) override;
  bool onTextInput(const char *text, ToolContext &ctx) override;

  void deactivate() override;

  // Wired to the existing ToolSettings::backgroundMode field, same one
  // Selection tools already share per Editor.h's comment on that field.
  void setBackgroundMode(ToolSettings::BackgroundMode mode) { m_bgMode = mode; }

  Style &style() { return m_style; }
};
