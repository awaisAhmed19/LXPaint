#pragma once
class Canvas;

class Command {
public:
  virtual ~Command() {}
  virtual void execute(Canvas &c) = 0;
  virtual void undo(Canvas &c) = 0;
};
