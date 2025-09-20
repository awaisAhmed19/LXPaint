#include <iostream>

#include "App.h"

using namespace std;
App* app = 0;

int main(int argc, char* argv[]) {
    app = new App();

    app->init("LXPaint", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 650, 650, false);
    while (app->running()) {
        app->handleEvents();
        app->update();
        app->render();
    }

    app->clean();

    return 0;
}
