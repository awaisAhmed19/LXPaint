#include <cmath>

#include "Globals.h"

namespace MATH {

bool MouseOverPoint(vec2 target, vec2 mouse) {
  float hoverDistance = 10.0f;
  float distance = std::sqrtf(std::powf(mouse.x - target.x, 2) +
                              std::powf(mouse.y - target.y, 2));
  return distance <= hoverDistance;
}
}; // namespace MATH
