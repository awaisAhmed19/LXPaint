#pragma once

#include "App/Globals.h"
#include "Document/Canvas.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Tools/ClickTool.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Eyedropper  (ClickTool)
//
//  Left-click: samples the canvas pixel under the cursor and writes it into
//  ctx.fgColorOut, then flags ctx.colorSampledOut so the App sync loop knows
//  to propagate the new color into the ColorPalette this frame.
//
// ─────────────────────────────────────────────────────────────────────────────

class Eyedropper : public ClickTool {
public:
  std::unique_ptr<Command> onMouseClick(vec2 pos, ToolContext &ctx) override;
};
