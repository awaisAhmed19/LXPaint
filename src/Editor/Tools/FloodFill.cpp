
#include "FloodFill.h"

#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/Interaction/ToolContext.h"
// #include "Editor/Interaction/ToolInteractionState.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"
#include <memory>
std::unique_ptr<Command> FloodFill::onMouseClick(vec2 pos, ToolContext &ctx) {

  LX_ASSERT(ctx.canvas != nullptr, "FloodFill canvas missing");
  LX_ASSERT(ctx.interaction != nullptr, "FloodFill interaction missing");
  Logger::debug("FloodFill START");

  SDL_Rect initialRect;

  int h = ctx.canvas->getHeight();
  int w = ctx.canvas->getWidth();

  auto before =
      SnapshotCommand::copyRegion(ctx.canvas->getSurface(), {0, 0, w, h});

  Rasterizer::floodFill(ctx.canvas->getSurface(), pos, ctx.fgColor);
  auto after =
      SnapshotCommand::copyRegion(ctx.canvas->getSurface(), {0, 0, w, h});
  initialRect = {0, 0, ctx.canvas->getWidth(), ctx.canvas->getHeight()};

  ctx.canvas->invalidateRect(initialRect);
  // comment this block
  /*
  auto *surface = ctx.canvas->getSurface();
  uint32_t *pixels = (uint32_t *)surface->pixels;

  int pitch = surface->pitch / 4;

  int x = (int)pos.x;
  int y = (int)pos.y;

  std::cout << "Pixel after fill = %08x" << pixels[y * pitch + x] << std::endl;
*/
  // comment this block
  if (!before || !after) {
    Logger::log(LogLevel::ERR, "Snapshot region capture failed");
    return nullptr;
  }
  auto cmd = std::make_unique<SnapshotCommand>(std::move(before),
                                               std::move(after), initialRect);
  Logger::debug("FloodFill END");
  return cmd;
}
