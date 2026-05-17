#include "../../App/Globals.h"
struct ToolInteractionState {
  bool active = false;

  vec2 startMousePos{0.0f, 0.0f};
  vec2 currMousePos{0.0f, 0.0f};
  vec2 prevMousePos{0.0f, 0.0f};

  bool mouseDown = false;

  SDL_Keymod modifier = SDL_KMOD_NONE;
  uint64_t interactionID = 0;
};
