#include "SelectionTool.h"
#include "Editor/Commands/SnapshotCommand.h"
#include "Rendering/Rasterizer.h"

#include "Document/PreviewLayer.h"
SelectionTool::~SelectionTool() { clearSelection(); }

void SelectionTool::copyFromCanvas(SDL_Surface *canvas) {
  if (m_selection.empty())
    return;

  if (m_selection.pixels) {
    SDL_DestroySurface(m_selection.pixels);
    m_selection.pixels = nullptr;
  }

  m_selection.pixels = SDL_CreateSurface(m_selection.bounds.w,
                                         m_selection.bounds.h, canvas->format);
  LX_ASSERT(m_selection.pixels != nullptr,
            "Failed to create selection surface.");
  SDL_FillSurfaceRect(m_selection.pixels, nullptr, 0x00000000);
  auto *src = static_cast<Uint32 *>(canvas->pixels);
  auto *dst = static_cast<Uint32 *>(m_selection.pixels->pixels);
  int srcPitch = canvas->pitch / sizeof(Uint32);
  int dstPitch = m_selection.pixels->pitch / sizeof(Uint32);

  for (int y = 0; y < m_selection.bounds.h; ++y) {
    for (int x = 0; x < m_selection.bounds.w; ++x) {
      if (!m_selection.mask[y * m_selection.bounds.w + x])
        continue;
      int sx = m_selection.bounds.x + x;
      int sy = m_selection.bounds.y + y;
      dst[y * dstPitch + x] = src[sy * srcPitch + sx];
    }
  }
}
void SelectionTool::selectAllCanvas(ToolContext &ctx) {
  int w = ctx.canvas->getWidth();
  int h = ctx.canvas->getHeight();

  m_selection.bounds = {0, 0, w, h};
  m_selection.mask.assign(w * h, 1);
  m_selection.offset = {0.f, 0.f};

  copyFromCanvas(ctx.canvas->getSurface());
  m_state = SelectionState::Selected;
}

void SelectionTool::clearSelection() {
  if (m_selection.pixels) {
    SDL_DestroySurface(m_selection.pixels);
    m_selection.pixels = nullptr;
  }
  m_selection.mask.clear();
  m_selection.bounds = {0, 0, 0, 0};
  m_selection.offset = {0.f, 0.f};
  m_state = SelectionState::Idle;
}

std::unique_ptr<Command> SelectionTool::commitSelection(ToolContext &ctx) {
  if (m_selection.empty())
    return nullptr;

  int offsetX = (int)m_selection.offset.x;
  int offsetY = (int)m_selection.offset.y;

  if (offsetX == 0 && offsetY == 0)
    return nullptr;

  SDL_Rect src = m_selection.bounds;
  SDL_Rect dstRect = {m_selection.bounds.x + offsetX,
                      m_selection.bounds.y + offsetY, m_selection.bounds.w,
                      m_selection.bounds.h};

  SDL_Rect dirty = {std::min(src.x, dstRect.x), std::min(src.y, dstRect.y),
                    std::max(src.x + src.w, dstRect.x + dstRect.w) -
                        std::min(src.x, dstRect.x),
                    std::max(src.y + src.h, dstRect.y + dstRect.h) -
                        std::min(src.y, dstRect.y)};

  auto snapshot =
      std::make_unique<SnapshotCommand>(ctx.canvas->getSurface(), dirty);
  SDL_Surface *canvas = ctx.canvas->getSurface();
  auto *pixels = static_cast<Uint32 *>(canvas->pixels);
  int dstPitch = canvas->pitch / sizeof(Uint32);

  for (int y = 0; y < m_selection.bounds.h; ++y) {
    for (int x = 0; x < m_selection.bounds.w; ++x) {
      if (!m_selection.mask[y * m_selection.bounds.w + x])
        continue;
      int cx = m_selection.bounds.x + x;
      int cy = m_selection.bounds.y + y;
      if (cx < 0 || cx >= canvas->w || cy < 0 || cy >= canvas->h)
        continue;
      pixels[cy * dstPitch + cx] = ctx.bgColor;
    }
  }

  if (m_selection.pixels) {
    auto *srcPixels = static_cast<Uint32 *>(m_selection.pixels->pixels);
    int srcPitch = m_selection.pixels->pitch / sizeof(Uint32);

    for (int y = 0; y < m_selection.bounds.h; ++y) {
      for (int x = 0; x < m_selection.bounds.w; ++x) {
        if (!m_selection.mask[y * m_selection.bounds.w + x])
          continue;

        int dx = m_selection.bounds.x + x + offsetX;
        int dy = m_selection.bounds.y + y + offsetY;

        if (dx < 0 || dx >= canvas->w || dy < 0 || dy >= canvas->h)
          continue;

        pixels[dy * dstPitch + dx] = srcPixels[y * srcPitch + x];
      }
    }
  }

  snapshot->captureAfter(ctx.canvas->getSurface());
  ctx.canvas->invalidateRect(dirty);
  return snapshot;
}

void SelectionTool::drawSelectionOutline(ToolContext &ctx) {
  if (m_selection.empty())
    return;
  SDL_Rect b = m_selection.bounds;
  Rasterizer::drawRectStroke(
      ctx.preview->getSurface(), {(float)b.x, (float)b.y},
      {(float)(b.x + b.w), (float)(b.y + b.h)}, 0xFFFFFFFF, 1);
  ctx.preview->invalidateRect(b);
}
