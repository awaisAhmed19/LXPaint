#pragma once

#include "FooterMessages.h"
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  HoverStatus
//
//  A frame-scoped, single-writer status channel between any UI widget and
//  the Footer.
//
//  Design rules
//  ────────────
//  • Any UI widget calls HoverStatus::push(key) when ImGui::IsItemHovered().
//  • Exactly ONE call per frame wins (first writer per frame takes precedence,
//    matching standard Win95 behaviour where the topmost hovered item wins).
//  • Footer::render() calls HoverStatus::current() to read the message, then
//    calls HoverStatus::endFrame() at the end of its render pass to reset for
//    the next frame.
//  • No widget needs to call "clear" — the reset is automatic each frame.
//
//  One-liner widget integration
//  ────────────────────────────
//    ImGui::InvisibleButton(...);
//    if (ImGui::IsItemHovered())
//        HoverStatus::push(FooterMessages::Key::Pencil);
//
//  That is the complete integration cost for any new control.
// ─────────────────────────────────────────────────────────────────────────────

class HoverStatus {
public:
  // Called by any UI widget that is currently hovered.
  // key  — a FooterMessages::Key constant or raw lookup key string.
  // Only the first push per frame takes effect.
  static void push(const char *key) {
    if (s_lockedThisFrame)
      return;
    const std::string *msg = FooterMessages::get(key);
    if (msg) {
      s_current = msg;
      s_lockedThisFrame = true;
    }
  }

  // Same as push() but accepts a std::string key (for dynamic keys).
  static void push(const std::string &key) { push(key.c_str()); }

  // Returns the message to display, or nullptr when nothing is hovered.
  // Footer::render() calls this to decide what text to show.
  static const std::string *current() { return s_current; }

  // Must be called once per frame after Footer has consumed the message.
  // Footer::render() calls this at the end of its own render().
  static void endFrame() {
    s_current = nullptr;
    s_lockedThisFrame = false;
  }

private:
  inline static const std::string *s_current = nullptr;
  inline static bool s_lockedThisFrame = false;
};
