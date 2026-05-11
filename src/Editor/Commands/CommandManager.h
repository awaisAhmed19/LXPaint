
#include "Command.h"
#include <memory>
#include <stack>
class CommandManager {
private:
  std::stack<std::unique_ptr<Command>> undoStack;
  std::stack<std::unique_ptr<Command>> redoStack;

public:
  void executeCommand(std::unique_ptr<Command> cmd, Canvas &canvas) {
    cmd->execute(canvas);
    undoStack.push(std::move(cmd)); // Remember the work

    // Clear redo history because a new path was taken
    while (!redoStack.empty()) {
      redoStack.pop();
    }
  }

  void undo(Canvas &canvas) {
    if (undoStack.empty())
      return;

    std::unique_ptr<Command> cmd = std::move(undoStack.top());
    undoStack.pop();

    cmd->undo(canvas);              // Reverse the work
    redoStack.push(std::move(cmd)); // Move it to redo in case we want it back
  }

  void redo(Canvas &canvas) {
    if (redoStack.empty())
      return;

    std::unique_ptr<Command> cmd = std::move(redoStack.top());
    redoStack.pop();

    cmd->execute(canvas);           // Re-do the work
    undoStack.push(std::move(cmd)); // Put it back in undo history
  }
};
