#include "../../App/Globals.h"
struct ToolInteractionState {
  bool active = false;

  vec2 startMousePos{0};
  vec2 currMousePos{0};
  vec2 prevMousePos{0};

  bool mouseDown = false;

  SDL_Keymod modifier = SDL_KMOD_NONE;
  uint64_t interactionID = 0;
};
