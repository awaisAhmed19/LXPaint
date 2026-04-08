#pragma once
class Canvas;
class Command {
public:
    virtual ~Command() {}
    virtual void execute(Canvas& canvas) = 0;
    virtual void undo(Canvas& canvas) = 0;
};
