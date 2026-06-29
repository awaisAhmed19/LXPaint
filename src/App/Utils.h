#include <cmath>

#include "Globals.h"

namespace MATH {

inline bool MouseOverPoint(vec2 target, vec2 mouse) {
  float hoverDistance = 10.0f;
  float dx = mouse.x - target.x;
  float dy = mouse.y - target.y;

  float distance = std::sqrt(dx * dx + dy * dy);
  return distance <= hoverDistance;
}
}; // namespace MATH
