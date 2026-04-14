#include "App.h"
LineAlgo g_CurrentAlgo = LineAlgo::BRESENHAM;
int main(int argc, char *argv[]) {
  App app("LXPAINT");
  app.run();
  return 0;
}
