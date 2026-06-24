#include "Text.h"

#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Editor/ToolSettings.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"

#include <algorithm>
#include <cstring>

TTF_Font *Text::s_font = nullptr;

// ─────────────────────────────────────────────────────────────
//  Font
// ─────────────────────────────────────────────────────────────

bool Text::initFontSystem(const std::string &fontPath, int defaultPointSize) {
  if (s_font) {
    // Already initialized — App::Application is expected to call this
    // exactly once at startup; a second call is treated as a no-op rather
    // than leaking a second TTF_Font (would also fail to update s_font
    // safely if a Text instance is mid-render on another thread, though
    // this codebase is single-threaded for rendering so that's moot today).
    Logger::log(LogLevel::WARNING,
                "Text::initFontSystem called more than once — ignoring");
    return true;
  }

  s_font = TTF_OpenFont(fontPath.c_str(), defaultPointSize);

  if (!s_font) {
    Logger::log(LogLevel::ERR,
                std::format("Text::initFontSystem: TTF_OpenFont('{}') "
                            "failed: {}",
                            fontPath, SDL_GetError()));
    return false;
  }

  Logger::debug(std::format("Text::initFontSystem: loaded '{}' @ {}pt",
                            fontPath, defaultPointSize));
  return true;
}

void Text::shutdownFontSystem() {
  if (s_font) {
    TTF_CloseFont(s_font);
    s_font = nullptr;
    Logger::debug("Text::shutdownFontSystem: font closed");
  }
}

bool Text::ensureFont(int pointSize) {
  // No opening happens here anymore — initFontSystem (called once by
  // App::Application at startup) is the only place that calls
  // TTF_OpenFont. If s_font is null, Application either hasn't started up
  // yet or initFontSystem failed (already logged there); either way,
  // there is nothing this function can correctly do about it, so it just
  // reports failure rather than silently opening a second, differently-
  // owned font behind Application's back.
  if (!s_font) {
    return false;
  }

  // Only the default font/size is guaranteed functional per spec; if a
  // different size is requested we still try to honor it via
  // TTF_SetFontSize rather than reloading, since SDL_ttf supports resizing
  // a loaded font in place and reloading would require knowing the
  // original path, which Text no longer holds (Application does).
  if (!TTF_SetFontSize(s_font, pointSize)) {
    Logger::log(LogLevel::ERR,
                std::format("TTF_SetFontSize failed: {}", SDL_GetError()));
  }
  return true;
}

Text::~Text() {
  // s_font is owned by Application (see initFontSystem/shutdownFontSystem)
  // and shared read-only across every Text instance — nothing to free
  // here. Per-instance teardown that previously lived here was never
  // correct anyway, since multiple Text activations would have raced to
  // free the same shared pointer.
}

// ─────────────────────────────────────────────────────────────
//  Measurement
// ─────────────────────────────────────────────────────────────

int Text::lineHeight() const {
  if (!s_font)
    return m_style.pointSize + 4;
  return TTF_GetFontHeight(s_font);
}

int Text::textWidth(const char *utf8Start, size_t byteLen) const {
  if (!s_font || byteLen == 0)
    return 0;

  // TTF_GetStringSize measures a UTF-8 buffer that need not be
  // NUL-terminated as long as byteLen is given explicitly (SDL3_ttf takes
  // a length parameter precisely so callers don't need to allocate a
  // temporary substring just to measure it).
  int w = 0, h = 0;
  if (!TTF_GetStringSize(s_font, utf8Start, byteLen, &w, &h)) {
    Logger::log(LogLevel::ERR,
                std::format("TTF_GetStringSize failed: {}", SDL_GetError()));
    return 0;
  }
  return w;
}

// ─────────────────────────────────────────────────────────────
//  Word wrap
// ─────────────────────────────────────────────────────────────

void Text::wrapBuffer(int wrapWidth) {
  m_wrapCache.clear();

  if (wrapWidth <= 0) {
    m_wrapDirty = false;
    return;
  }

  const char *data = m_buffer.data();
  size_t len = m_buffer.size();

  int y = 0;
  const int lh = lineHeight();

  size_t paraStart = 0;

  // Split into paragraphs on '\n', wrap each paragraph independently.
  for (size_t i = 0; i <= len; ++i) {
    bool atParagraphEnd = (i == len) || (data[i] == '\n');
    if (!atParagraphEnd)
      continue;

    size_t paraEnd = i; // exclusive, does not include the '\n' itself

    size_t lineStart = paraStart;
    size_t lastSpace = std::string::npos;
    size_t cursor = paraStart;

    if (paraStart == paraEnd) {
      // Empty paragraph (blank line, or trailing newline) — still emit one
      // empty visual line so blank lines remain visible and caret-
      // addressable.
      m_wrapCache.push_back({paraStart, paraEnd, y});
      y += lh;
    } else {
      while (cursor < paraEnd) {
        // Advance to the next space or paragraph end.
        size_t next = cursor;
        while (next < paraEnd && data[next] != ' ')
          ++next;
        if (next < paraEnd) {
          lastSpace = next;
          ++next; // step past the space itself
        }

        int w = textWidth(data + lineStart, next - lineStart);

        if (w > wrapWidth && next > lineStart + 1) {
          if (lastSpace != std::string::npos && lastSpace > lineStart) {
            m_wrapCache.push_back({lineStart, lastSpace, y});
            y += lh;
            lineStart = lastSpace + 1;
            lastSpace = std::string::npos;
            cursor = lineStart;
            continue;
          }

          // Single "word" (no space) wider than wrapWidth — hard-break by
          // walking back character-by-character until it fits, so at
          // least one character is always placed per line (avoids an
          // infinite loop on a width too small for even one glyph).
          size_t breakAt = lineStart + 1;
          while (breakAt < next &&
                 textWidth(data + lineStart, breakAt - lineStart) <=
                     wrapWidth) {
            ++breakAt;
          }
          if (breakAt > lineStart + 1)
            --breakAt; // step back to the last fitting position

          m_wrapCache.push_back({lineStart, breakAt, y});
          y += lh;
          lineStart = breakAt;
          lastSpace = std::string::npos;
          cursor = lineStart;
          continue;
        }

        cursor = next;
      }

      // Remainder of the paragraph after the wrap loop.
      m_wrapCache.push_back({lineStart, paraEnd, y});
      y += lh;
    }

    paraStart = i + 1; // skip past the '\n'
  }

  if (m_wrapCache.empty()) {
    // Buffer is completely empty — still need one addressable line so the
    // caret has somewhere to sit and the caret-blink draw has a position.
    m_wrapCache.push_back({0, 0, 0});
  }

  m_wrapDirty = false;
}

void Text::rebuildWrapIfNeeded() {
  if (m_wrapDirty) {
    wrapBuffer(std::max(1, m_rect.w));
  }
}

// ─────────────────────────────────────────────────────────────
//  Caret
// ─────────────────────────────────────────────────────────────

size_t Text::wrapLineForCaret() const {
  for (size_t i = 0; i < m_wrapCache.size(); ++i) {
    const auto &line = m_wrapCache[i];
    bool isLast = (i + 1 == m_wrapCache.size());
    // Caret exactly at line.end belongs to this line unless there's a
    // following line that starts there too (i.e. don't bleed into the
    // next line just because the caret sits at a boundary byte).
    if (m_caret >= line.start &&
        (m_caret < line.end || isLast || m_caret < m_wrapCache[i + 1].start)) {
      return i;
    }
  }
  return m_wrapCache.empty() ? 0 : m_wrapCache.size() - 1;
}

void Text::moveCaretVertical(int dLines) {
  rebuildWrapIfNeeded();
  if (m_wrapCache.empty())
    return;

  size_t curLine = wrapLineForCaret();
  size_t column = m_caret - m_wrapCache[curLine].start;

  long target = (long)curLine + dLines;
  target = std::clamp(target, 0L, (long)m_wrapCache.size() - 1);

  const auto &line = m_wrapCache[(size_t)target];
  size_t lineLen = line.end - line.start;

  m_caret = line.start + std::min(column, lineLen);
}

void Text::moveCaretHome() {
  rebuildWrapIfNeeded();
  if (m_wrapCache.empty())
    return;
  size_t curLine = wrapLineForCaret();
  m_caret = m_wrapCache[curLine].start;
}

void Text::moveCaretEnd() {
  rebuildWrapIfNeeded();
  if (m_wrapCache.empty())
    return;
  size_t curLine = wrapLineForCaret();
  m_caret = m_wrapCache[curLine].end;
}

// ─────────────────────────────────────────────────────────────
//  Buffer mutation
// ─────────────────────────────────────────────────────────────

void Text::insertText(const char *utf8) {
  size_t n = std::strlen(utf8);
  if (n == 0)
    return;

  m_buffer.insert(m_caret, utf8, n);
  m_caret += n;
  m_wrapDirty = true;
}

void Text::deleteBackward() {
  if (m_caret == 0)
    return;

  // Step back one UTF-8 code point (not just one byte) so backspace never
  // leaves a dangling continuation byte that would corrupt rendering.
  size_t start = m_caret - 1;
  while (start > 0 && ((unsigned char)m_buffer[start] & 0xC0) == 0x80) {
    --start;
  }

  m_buffer.erase(start, m_caret - start);
  m_caret = start;
  m_wrapDirty = true;
}

void Text::deleteForward() {
  if (m_caret >= m_buffer.size())
    return;

  size_t end = m_caret + 1;
  while (end < m_buffer.size() &&
         ((unsigned char)m_buffer[end] & 0xC0) == 0x80) {
    ++end;
  }

  m_buffer.erase(m_caret, end - m_caret);
  m_wrapDirty = true;
}

// ─────────────────────────────────────────────────────────────
//  Rendering
// ─────────────────────────────────────────────────────────────

// NOTE: renderToSurface was removed — redrawPreview() and commit() each
// render directly (background fill, then glyph blits, then caret/outline)
// using the real ctx.fgColor/ctx.bgColor. An earlier draft tried to factor
// this into a shared helper but ended up passing a placeholder color since
// the helper had no access to ToolContext; duplicating the ~15 lines in
// each of the two call sites was clearer than threading colors through a
// third function, given there are only two call sites.

void Text::redrawPreview(ToolContext &ctx) {
  ctx.preview->clearRGBA(0, 0, 0, 0);

  if (m_state != State::Editing) {
    ctx.preview->markDirty();
    return;
  }

  rebuildWrapIfNeeded();

  SDL_Surface *surf = ctx.preview->getSurface();

  if (m_bgMode == ToolSettings::BackgroundMode::Opaque) {
    Rasterizer::rectFill(surf, m_rect.x, m_rect.y, m_rect.x + m_rect.w - 1,
                         m_rect.y + m_rect.h - 1, ctx.bgColor);
  }

  // s_font may be null if Application::initFontSystem failed or hasn't run
  // yet — render the rect/background/caret/outline regardless (so editing
  // still visually works as an empty box) but skip glyph rendering, rather
  // than calling TTF_RenderText_Blended(nullptr, ...) which is unsafe.
  if (s_font) {
    for (const auto &line : m_wrapCache) {
      size_t lineLen = line.end - line.start;
      if (lineLen == 0)
        continue;

      SDL_Color fgSdl;
      fgSdl.a = (uint8_t)((ctx.fgColor >> 24) & 0xFF);
      fgSdl.r = (uint8_t)((ctx.fgColor >> 16) & 0xFF);
      fgSdl.g = (uint8_t)((ctx.fgColor >> 8) & 0xFF);
      fgSdl.b = (uint8_t)(ctx.fgColor & 0xFF);

      SDL_Surface *glyphs = TTF_RenderText_Blended(
          s_font, m_buffer.data() + line.start, lineLen, fgSdl);

      if (!glyphs)
        continue;

      SDL_Rect dst{m_rect.x, m_rect.y + line.y, glyphs->w, glyphs->h};
      SDL_BlitSurface(glyphs, nullptr, surf, &dst);
      SDL_DestroySurface(glyphs);
    }
  }

  // Caret — blinking is not implemented (no per-frame timer hook exists in
  // BaseTool/ToolContext), so it is drawn solid every frame. Acceptable per
  // spec ("Caret is visible" — blink is a polish detail, not a requirement).
  size_t curLine = wrapLineForCaret();
  if (curLine < m_wrapCache.size()) {
    const auto &line = m_wrapCache[curLine];
    size_t col = m_caret - line.start;
    int caretX = m_rect.x + textWidth(m_buffer.data() + line.start, col);
    int caretY = m_rect.y + line.y;
    Rasterizer::bresenham({(float)caretX, (float)caretY},
                          {(float)caretX, (float)(caretY + lineHeight())}, surf,
                          ctx.fgColor, 1, false);
  }

  // Selection rectangle (the editing box outline itself).
  Rasterizer::drawRectStroke(
      surf, {(float)m_rect.x, (float)m_rect.y},
      {(float)(m_rect.x + m_rect.w), (float)(m_rect.y + m_rect.h)}, 0xFF808080u,
      1);

  ctx.preview->invalidateRect(m_rect);
}

// ─────────────────────────────────────────────────────────────
//  State transitions
// ─────────────────────────────────────────────────────────────

bool Text::pointInRect(vec2 pos) const {
  return pos.x >= m_rect.x && pos.x < m_rect.x + m_rect.w &&
         pos.y >= m_rect.y && pos.y < m_rect.y + m_rect.h;
}

void Text::enterEditing(ToolContext &ctx) {
  m_state = State::Editing;
  m_buffer.clear();
  m_caret = 0;
  m_wrapDirty = true;
  m_selectAll = false;

  if (!ensureFont(m_style.pointSize)) {
    Logger::log(LogLevel::ERR,
                "Text: font unavailable, editing disabled this activation");
  }

  m_window = ctx.window;
  SDL_StartTextInput(m_window);

  redrawPreview(ctx);
}

std::unique_ptr<Command> Text::commit(ToolContext &ctx) {
  SDL_StopTextInput(m_window);

  if (m_buffer.empty()) {
    // Nothing typed — nothing to commit, no history entry.
    ctx.preview->clearRGBA(0, 0, 0, 0);
    ctx.preview->invalidateRect(m_rect);
    m_state = State::Idle;
    return nullptr;
  }

  rebuildWrapIfNeeded();

  SDL_Surface *canvasSurf = ctx.canvas->getSurface();

  m_command = std::make_unique<SnapshotCommand>(canvasSurf, m_rect);

  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->invalidateRect(m_rect);

  if (m_bgMode == ToolSettings::BackgroundMode::Opaque) {
    Rasterizer::rectFill(canvasSurf, m_rect.x, m_rect.y,
                         m_rect.x + m_rect.w - 1, m_rect.y + m_rect.h - 1,
                         ctx.bgColor);
  }

  // Same s_font-may-be-null guard as redrawPreview — commit still produces
  // a valid (if textless) SnapshotCommand rather than crashing.
  if (s_font) {
    for (const auto &line : m_wrapCache) {
      size_t lineLen = line.end - line.start;
      if (lineLen == 0)
        continue;

      SDL_Color fgSdl;
      fgSdl.a = (uint8_t)((ctx.fgColor >> 24) & 0xFF);
      fgSdl.r = (uint8_t)((ctx.fgColor >> 16) & 0xFF);
      fgSdl.g = (uint8_t)((ctx.fgColor >> 8) & 0xFF);
      fgSdl.b = (uint8_t)(ctx.fgColor & 0xFF);

      SDL_Surface *glyphs = TTF_RenderText_Blended(
          s_font, m_buffer.data() + line.start, lineLen, fgSdl);

      if (!glyphs)
        continue;

      SDL_Rect dst{m_rect.x, m_rect.y + line.y, glyphs->w, glyphs->h};
      SDL_BlitSurface(glyphs, nullptr, canvasSurf, &dst);
      SDL_DestroySurface(glyphs);
    }
  }

  ctx.canvas->invalidateRect(m_rect);
  m_command->captureAfter(canvasSurf);

  m_state = State::Idle;
  m_buffer.clear();

  return std::move(m_command);
}

void Text::cancel(ToolContext &ctx) {
  SDL_StopTextInput(ctx.window);

  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->invalidateRect(m_rect);

  m_state = State::Idle;
  m_buffer.clear();
  m_caret = 0;
  m_rect = {0, 0, 0, 0};
}

// ─────────────────────────────────────────────────────────────
//  Tool interface
// ─────────────────────────────────────────────────────────────

void Text::onMouseDown(vec2 pos, ToolContext &ctx) {
  if (m_state == State::Idle) {
    m_dragStart = pos;
    m_rect = {(int)pos.x, (int)pos.y, 0, 0};
    m_state = State::Placing;
    return;
  }

  if (m_state == State::Editing) {
    if (pointInRect(pos)) {
      // Click inside — move the caret to the nearest character. Simplified
      // hit-test: find the wrap line whose y-range contains the click,
      // then walk that line's glyphs to find the closest column. Mouse
      // text *selection* is out of scope, but click-to-place-caret is
      // explicitly required ("Clicking inside moves the caret").
      rebuildWrapIfNeeded();

      int localY = (int)pos.y - m_rect.y;
      int localX = (int)pos.x - m_rect.x;

      size_t bestLine = 0;
      for (size_t i = 0; i < m_wrapCache.size(); ++i) {
        if (m_wrapCache[i].y <= localY)
          bestLine = i;
      }

      const auto &line = m_wrapCache[bestLine];
      size_t lineLen = line.end - line.start;

      // Walk forward measuring cumulative width until it exceeds localX,
      // then back off one character — linear but lines are short in
      // practice and this avoids needing per-glyph advance tables.
      size_t col = lineLen;
      for (size_t c = 0; c <= lineLen; ++c) {
        int w = textWidth(m_buffer.data() + line.start, c);
        if (w > localX) {
          col = c > 0 ? c - 1 : 0;
          break;
        }
      }

      m_caret = line.start + col;
      m_selectAll = false;
      redrawPreview(ctx);
      return;
    }

    // Click outside the rect while editing — commit. onMouseDown can't
    // return a Command (only onMouseUp can, and Editor only pushes from
    // there — see Polygon for the same constraint), so arm a flag and
    // perform the actual commit on the matching onMouseUp for this same
    // click, stashing the result in m_pendingCommit for Editor to collect.
    m_commitArmedFromOutsideClick = true;
    return;
  }
}

void Text::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (m_state != State::Placing)
    return;

  int left = std::min((int)m_dragStart.x, (int)pos.x);
  int top = std::min((int)m_dragStart.y, (int)pos.y);
  int right = std::max((int)m_dragStart.x, (int)pos.x);
  int bottom = std::max((int)m_dragStart.y, (int)pos.y);

  m_rect = {left, top, right - left, bottom - top};

  ctx.preview->clearRGBA(0, 0, 0, 0);
  Rasterizer::drawRectStroke(ctx.preview->getSurface(),
                             {(float)left, (float)top},
                             {(float)right, (float)bottom}, ctx.fgColor, 1);
  ctx.preview->invalidateRect(m_rect);
}

std::unique_ptr<Command> Text::onMouseUp(vec2 pos, ToolContext &ctx) {
  if (m_state == State::Placing) {
    if (m_rect.w <= 2 || m_rect.h <= 2) {
      // Degenerate drag (effectively a click, not a drag) — discard.
      ctx.preview->clearRGBA(0, 0, 0, 0);
      ctx.preview->invalidateRect(m_rect);
      m_state = State::Idle;
      m_rect = {0, 0, 0, 0};
      return nullptr;
    }

    enterEditing(ctx);
    return nullptr;
  }

  if (m_commitArmedFromOutsideClick) {
    m_commitArmedFromOutsideClick = false;
    return commit(ctx);
  }

  return nullptr;
}

bool Text::onKeyDown(SDL_Scancode scancode, ToolContext &ctx) {
  if (m_state != State::Editing)
    return false;

  const SDL_Keymod mod = SDL_GetModState();
  const bool ctrlHeld = (mod & SDL_KMOD_CTRL) != 0;

  switch (scancode) {
  case SDL_SCANCODE_ESCAPE:
    cancel(ctx);
    return true;

  case SDL_SCANCODE_RETURN:
  case SDL_SCANCODE_KP_ENTER:
    if (ctrlHeld) {
      // Ctrl+Enter commits. onKeyDown returns bool, not a Command, so the
      // result is stashed in m_pendingCommit for Editor::handleEvent to
      // retrieve via takePendingCommit() immediately after this call
      // returns — see the header comment on m_pendingCommit.
      m_pendingCommit = commit(ctx);
      return true;
    }
    insertText("\n");
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_BACKSPACE:
    deleteBackward();
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_DELETE:
    deleteForward();
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_LEFT:
    if (m_caret > 0) {
      size_t p = m_caret - 1;
      while (p > 0 && ((unsigned char)m_buffer[p] & 0xC0) == 0x80)
        --p;
      m_caret = p;
    }
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_RIGHT:
    if (m_caret < m_buffer.size()) {
      size_t p = m_caret + 1;
      while (p < m_buffer.size() && ((unsigned char)m_buffer[p] & 0xC0) == 0x80)
        ++p;
      m_caret = p;
    }
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_UP:
    moveCaretVertical(-1);
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_DOWN:
    moveCaretVertical(1);
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_HOME:
    moveCaretHome();
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_END:
    moveCaretEnd();
    m_selectAll = false;
    redrawPreview(ctx);
    return true;

  case SDL_SCANCODE_A:
    if (ctrlHeld) {
      m_selectAll = true;
      return true;
    }
    return false; // let onTextInput handle plain 'a'

  case SDL_SCANCODE_C:
    if (ctrlHeld) {
      if (m_selectAll) {
        SDL_SetClipboardText(m_buffer.c_str());
      }
      return true;
    }
    return false;

  case SDL_SCANCODE_X:
    if (ctrlHeld) {
      if (m_selectAll) {
        SDL_SetClipboardText(m_buffer.c_str());
        m_buffer.clear();
        m_caret = 0;
        m_wrapDirty = true;
        m_selectAll = false;
        redrawPreview(ctx);
      }
      return true;
    }
    return false;

  case SDL_SCANCODE_V:
    if (ctrlHeld) {
      char *clip = SDL_GetClipboardText();
      if (clip) {
        if (m_selectAll) {
          m_buffer.clear();
          m_caret = 0;
          m_selectAll = false;
        }
        insertText(clip);
        SDL_free(clip);
        redrawPreview(ctx);
      }
      return true;
    }
    return false;

  default:
    return false;
  }
}

bool Text::onTextInput(const char *text, ToolContext &ctx) {
  if (m_state != State::Editing)
    return false;

  if (m_selectAll) {
    m_buffer.clear();
    m_caret = 0;
    m_selectAll = false;
  }

  insertText(text);
  redrawPreview(ctx);
  return true;
}

void Text::deactivate() {
  // Switching tools mid-edit discards the in-progress text — no
  // ToolContext is available here to clear the preview directly, but the
  // next tool's first redraw/clearRGBA covers it. No history is created.
  // m_window is used (not a ctx.window we don't have) since
  // BaseTool::deactivate() takes no parameters — see the m_window comment
  // in Text.h for why this is cached rather than passed in.
  Logger::debug("Text: deactivated — discarding in-progress edit");
  if (m_window) {
    SDL_StopTextInput(m_window);
  }
  m_state = State::Idle;
  m_buffer.clear();
  m_caret = 0;
  m_rect = {0, 0, 0, 0};
  BaseTool::deactivate();
}
