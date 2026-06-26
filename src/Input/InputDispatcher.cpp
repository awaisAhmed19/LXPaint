
#include "InputDispatcher.h"

void InputDispatcher::beginFrame() {

  m_leftPressed = false;
  m_leftReleased = false;

  m_beginPan = false;
  m_endPan = false;

  m_zoomTriggered = false;

  m_mouseDelta = {0, 0};
}

void InputDispatcher::keyBinds(SDL_Scancode code, InputCommand cmd) {
  keyBindings[code] = cmd;
}

void InputDispatcher::bindActions(InputCommand cmd,
                                  std::function<void()> action) {
  actionMap[cmd] = action;
}

void InputDispatcher::execute(InputCommand cmd) {

  if (actionMap.contains(cmd)) {
    actionMap[cmd]();
  }
}

void InputDispatcher::update(const SDL_Event &e) {

  switch (e.type) {
  case SDL_EVENT_KEY_DOWN: {

    if (e.key.scancode == SDL_SCANCODE_SPACE) {
      m_spaceHeld = true;
    }
    if (e.type == SDL_EVENT_KEY_DOWN) {
      const bool ctrl = e.key.mod & SDL_KMOD_CTRL;
      const bool shift = e.key.mod & SDL_KMOD_SHIFT;

      if (ctrl && shift && e.key.scancode == SDL_SCANCODE_S) {
        execute(InputCommand::SAVE_AS);
        return;
      }

      if (ctrl) {
        auto it = keyBindings.find(e.key.scancode);

        if (it != keyBindings.end())
          execute(it->second);
      }
    }
    break;
  }

  case SDL_EVENT_KEY_UP: {

    if (e.key.scancode == SDL_SCANCODE_SPACE) {
      m_spaceHeld = false;
    }

    break;
  }

  case SDL_EVENT_MOUSE_BUTTON_DOWN: {

    if (e.button.button == SDL_BUTTON_LEFT) {

      m_leftPressed = true;
      m_leftDown = true;

      if (m_spaceHeld) {
        m_beginPan = true;
        m_panning = true;
      }
    }

    break;
  }

  case SDL_EVENT_MOUSE_BUTTON_UP: {

    if (e.button.button == SDL_BUTTON_LEFT) {
      m_leftReleased = true;
      m_leftDown = false;

      if (m_panning) {
        m_endPan = true;
        m_panning = false;
      }
    }

    break;
  }

  case SDL_EVENT_MOUSE_MOTION: {

    vec2 current = {(float)e.motion.x, (float)e.motion.y};
    m_mouseDelta = {current.x - m_mousePos.x, current.y - m_mousePos.y};
    m_mousePos = current;

    break;
  }

  case SDL_EVENT_MOUSE_WHEEL: {
    m_zoomTriggered = true;
    m_zoomFactor = e.wheel.y > 0 ? 1.1f : 0.9f;

    break;
  }

  default:
    break;
  }
}

bool InputDispatcher::isSpaceHeld() const { return m_spaceHeld; }
bool InputDispatcher::leftMousePressed() const { return m_leftPressed; }
bool InputDispatcher::leftMouseReleased() const { return m_leftReleased; }
bool InputDispatcher::leftMouseDown() const { return m_leftDown; }
vec2 InputDispatcher::getMouseScreenPos() const { return m_mousePos; }
vec2 InputDispatcher::getMouseDelta() const { return m_mouseDelta; }
bool InputDispatcher::beginPanRequested() const { return m_beginPan; }
bool InputDispatcher::endPanRequested() const { return m_endPan; }
bool InputDispatcher::isPanning() const { return m_panning; }
bool InputDispatcher::zoomTriggered() const { return m_zoomTriggered; }
float InputDispatcher::getZoomFactor() const { return m_zoomFactor; }
