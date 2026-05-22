#pragma once

#include <SDL3/SDL.h>

#include <functional>
#include <unordered_map>

#include "App/Globals.h"

enum class InputCommand {
    UNDO,
    REDO,
    PENCIL,
    ERASER,
    RECT,
    CIRCLE,
    LINE,
    FILL,
};

class InputDispatcher {
   public:
    void keyBinds(SDL_Scancode code, InputCommand cmd);
    void bindActions(InputCommand cmd, std::function<void()> action);
    void update(const SDL_Event& e);
    void beginFrame();

    /*
      Keyboard
    */

    bool isSpaceHeld() const;

    /*
      Mouse buttons
    */

    bool leftMousePressed() const;
    bool leftMouseReleased() const;
    bool leftMouseDown() const;

    /*
      Mouse movement
    */

    vec2 getMouseScreenPos() const;
    vec2 getMouseDelta() const;

    /*
      Panning
    */

    bool beginPanRequested() const;
    bool endPanRequested() const;
    bool isPanning() const;

    /*
      Zoom
    */

    bool zoomTriggered() const;
    float getZoomFactor() const;

   private:
    std::unordered_map<SDL_Scancode, InputCommand> keyBindings;

    std::unordered_map<InputCommand, std::function<void()>> actionMap;

   private:
    bool m_spaceHeld = false;

    bool m_leftPressed = false;
    bool m_leftReleased = false;
    bool m_leftDown = false;

    bool m_beginPan = false;
    bool m_endPan = false;
    bool m_panning = false;

    bool m_zoomTriggered = false;
    float m_zoomFactor = 1.0f;

    vec2 m_mousePos{};
    vec2 m_mouseDelta{};

   private:
    void execute(InputCommand cmd);
};
