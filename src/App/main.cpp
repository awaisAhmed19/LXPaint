#include "App.h"
#include "Globals.h"

#include <cstring>

#define APP_NAME "LXPAINT"

int main(int argc, char *argv[]) {

  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "--debug") == 0) {
      Config::ENABLE_DEBUG_LOGS = true;
      SDL_Log("Debug logs enabled");
    }

    if (std::strcmp(argv[i], "--assert") == 0) {
      Config::ENABLE_ASSERTS = true;
      SDL_Log("Assertions enabled");
    }
  }

  App app(APP_NAME);
  app.run();
  return 0;
}
