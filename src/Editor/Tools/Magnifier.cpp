#include "Magnifier.h"

#include "Editor/Interaction/ToolContext.h"
#include "Editor/Viewport/Viewport.h"
#include "Systems/Assert.h"
#include "Systems/Logger.h"
#include <algorithm>
#include <cmath>

int Magnifier::currentLevelIndex(float zoom) const {
  int best = 0;
  float bestDiff = std::abs(zoom - kLevels[0]);

  for (int i = 1; i < kLevelCount; ++i) {
    float diff = std::abs(zoom - kLevels[i]);
    if (diff < bestDiff) {
      bestDiff = diff;
      best = i;
    }
  }
  return best;
}

std::unique_ptr<Command> Magnifier::onMouseClick(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.viewport != nullptr, "Magnifier viewport missing");

  Viewport &vp = *ctx.viewport;

  float currentZoom = vp.getZoom();
  int idx = currentLevelIndex(currentZoom);

  int targetIdx;
  if (m_rightClick) {
    targetIdx = std::max(0, idx - 1);
  } else {
    targetIdx = std::min(kLevelCount - 1, idx + 1);
  }

  float targetZoom = (float)kLevels[targetIdx];
  float factor = targetZoom / currentZoom;

  // pos is already in canvas/document space (Editor passes the click-point
  // through screenToCanvas before calling tools), but ZoomAt expects a
  // *screen*-space anchor. Editor is responsible for passing the raw
  // screen-space mouse position here for Magnifier specifically — see
  // Editor::handleEvent change. If factor is ~1 (already at min/max), still
  // no-op safely since ZoomAt clamps internally.
  if (factor != 1.0f) {
    vp.ZoomAt(pos, factor);
    Logger::debug(
        std::format("Magnifier: zoom {} -> {}", currentZoom, targetZoom));
  }

  // No canvas mutation — no command, no undo entry.
  return nullptr;
}
