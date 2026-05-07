#include <SDL3/SDL.h>
#include <functional>
#include <unordered_map>

enum class InputCommand {
  UNDO,
  REDO,
  PENCIL,
  ERASER,
  RECT,
  LINE,
  FILL,
};

class InputDispatcher {
public:
  void keyBinds(SDL_Scancode code, InputCommand cmd);
  void bindActions(InputCommand cmd, std::function<void()> action);
  void updateKeyInput(SDL_Event e);

private:
  std::unordered_map<SDL_Scancode, InputCommand> keyBindings;
  std::unordered_map<InputCommand, std::function<void()>> actionMap;

  void execute(InputCommand cmd) {
    if (actionMap.contains(cmd)) {
      actionMap[cmd]();
    }
  }
};
