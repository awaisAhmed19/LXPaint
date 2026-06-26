
#pragma once

#include "Command.h"
#include "Editor/EditorDocument.h"
#include <deque>
#include <format>
#include <memory>
#include <string>
class CommandManager {
public:
  static constexpr size_t DEFAULT_MAX_UNDO = 50;
  static constexpr size_t DEFAULT_MAX_MEMORY_MB = 256;

private:
  struct CommandEntry {
    std::unique_ptr<Command> command;
    std::string description;

    CommandEntry(std::unique_ptr<Command> cmd, const std::string &desc)
        : command(std::move(cmd)), description(desc) {}
  };

private:
  std::deque<CommandEntry> m_undoStack;
  std::deque<CommandEntry> m_redoStack;
  size_t m_maxUndoDepth;
  size_t m_maxMemoryBytes;

public:
  explicit CommandManager(size_t maxUndoDepth = DEFAULT_MAX_UNDO,
                          size_t maxMemoryMB = DEFAULT_MAX_MEMORY_MB)
      : m_maxUndoDepth(maxUndoDepth),
        m_maxMemoryBytes(maxMemoryMB * 1024 * 1024) {}

public:
  bool pushCommand(std::unique_ptr<Command> cmd,
                   const std::string &description = "Action") {
    if (!cmd)
      return false;

    clearRedo();
    m_undoStack.emplace_back(std::move(cmd), description);

    trimUndoStack();
    return true;
  }
  size_t undoCount() const { return m_undoStack.size(); }

  size_t redoCount() const { return m_redoStack.size(); }

  float memoryUsageMB() const {
    return totalMemoryUsage() / (1024.0f * 1024.0f);
  }
  bool undo(Canvas &canvas) {
    if (m_undoStack.empty())
      return false;

    CommandEntry entry = std::move(m_undoStack.back());

    m_undoStack.pop_back();

    entry.command->undo(canvas);

    m_redoStack.push_back(std::move(entry));

    return true;
  }

  bool redo(Canvas &canvas) {
    if (m_redoStack.empty())
      return false;

    CommandEntry entry = std::move(m_redoStack.back());

    m_redoStack.pop_back();

    entry.command->redo(canvas);

    m_undoStack.push_back(std::move(entry));

    return true;
  }

public:
  bool canUndo() const { return !m_undoStack.empty(); }

  bool canRedo() const { return !m_redoStack.empty(); }

public:
  void clear() {
    m_undoStack.clear();
    m_redoStack.clear();
  }

  void clearRedo() { m_redoStack.clear(); }
  std::string getDebugInfo() const {
    return std::format("Undo: {} | Redo: {}", m_undoStack.size(),
                       m_redoStack.size());
  }

private:
  size_t totalMemoryUsage() const {
    size_t total = 0;

    for (const auto &e : m_undoStack)
      total += e.command->memoryUsage();

    for (const auto &e : m_redoStack)
      total += e.command->memoryUsage();

    return total;
  }

  void trimUndoStack() {
    while ((m_undoStack.size() > m_maxUndoDepth ||
            totalMemoryUsage() > m_maxMemoryBytes) &&
           !m_undoStack.empty()) {
      m_undoStack.pop_front();
    }
  }
};
