#include "InputDispatcher.h"

void InputDispatcher::keyBinds(SDL_Scancode code, InputCommand cmd) {
  keyBindings[code] = cmd;
}
void InputDispatcher::bindActions(InputCommand cmd,
                                  std::function<void()> action) {
  actionMap[cmd] = action;
}
void InputDispatcher::updateKeyInput(SDL_Event e) {
  if (e.type != SDL_EVENT_KEY_DOWN)
    return;
  if (e.key.mod & SDL_KMOD_CTRL) {
    auto keyIt = keyBindings.find(e.key.scancode);

    if (keyIt != keyBindings.end()) {

      InputCommand cmd = keyIt->second;

      auto actionIt = actionMap.find(cmd);

      if (actionIt != actionMap.end()) {
        actionIt->second();
      }
    }
  }
}
