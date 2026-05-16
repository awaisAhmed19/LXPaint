#pragma once
#include "../../Document/Canvas.h"
#include <cstddef>

class Command {
public:
  virtual ~Command() = default;

  virtual void undo(Canvas &canvas) = 0;
  virtual void redo(Canvas &canvas) = 0;

  virtual size_t memoryUsage() const = 0;
};
