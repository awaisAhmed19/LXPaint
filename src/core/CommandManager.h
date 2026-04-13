
#include "Command.h"
#include <stack>
class CommandManager {
private:
    std::stack<Command*> undoStack;
    std::stack<Command*> redoStack;

public:
    void executeCommand(Command* cmd, Canvas& canvas) {
        cmd->execute(canvas);  // Do the work
        undoStack.push(cmd);   // Remember the work

        // Clear redo history because a new path was taken
        while (!redoStack.empty()) {
            delete redoStack.top();
            redoStack.pop();
        }
    }

    void undo(Canvas& canvas) {
        if (undoStack.empty()) return;

        Command* cmd = undoStack.top();
        undoStack.pop();

        cmd->undo(canvas);     // Reverse the work
        redoStack.push(cmd);   // Move it to redo in case we want it back
    }

    void redo(Canvas& canvas) {
        if (redoStack.empty()) return;

        Command* cmd = redoStack.top();
        redoStack.pop();

        cmd->execute(canvas);  // Re-do the work
        undoStack.push(cmd);   // Put it back in undo history
    }
~CommandManager() {
    while (!undoStack.empty()) {
        delete undoStack.top();
        undoStack.pop();
    }
    while (!redoStack.empty()) {
        delete redoStack.top();
        redoStack.pop();
    }
}
};
