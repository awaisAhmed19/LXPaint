
#include "../../Systems/Logger.h"
#include "Command.h"
#include <cstddef>
#include <deque>
#include <memory>
#include <optional>
#include <string>
class CommandManager {
public:
  static constexpr size_t DEFAULT_MAX_UNDO = 50;
  static constexpr size_t DEFAULT_MAX_MEMORY_MB = 256;

private:
  struct CommandEntry {
    std::unique_ptr<Command> command;
    std::string description;
    size_t estimatedMemory = 0;

    CommandEntry(std::unique_ptr<Command> cmd, const std::string &desc,
                 size_t mem)
        : command(std::move(cmd)), description(desc), estimatedMemory(mem) {}
  };

  std::deque<CommandEntry> m_undoStack; // Changed from stack to deque
  std::deque<CommandEntry> m_redoStack; // Changed from stack to deque
  size_t m_maxUndoDepth;
  size_t m_maxMemoryBytes;
  size_t m_currentMemoryUsage = 0;

public:
  explicit CommandManager(size_t maxUndoDepth = DEFAULT_MAX_UNDO,
                          size_t maxMemoryMB = DEFAULT_MAX_MEMORY_MB)
      : m_maxUndoDepth(maxUndoDepth),
        m_maxMemoryBytes(maxMemoryMB * 1024 * 1024) {}

  // ====== MAIN OPERATIONS ======

  bool executeCommand(std::unique_ptr<Command> cmd, Canvas &canvas,
                      const std::string &description = "Action",
                      size_t estimatedMemory = 0) {
    if (!cmd)
      return false;

    // Execute immediately
    cmd->execute(canvas);
    canvas.markDirty();
    // Check memory limit
    if (m_currentMemoryUsage + estimatedMemory > m_maxMemoryBytes) {
      Logger::log(LogLevel::WARNING, "CommandManager: Memory limit exceeded");
      return false;
    }

    // Add to undo stack (at the back, like push)
    m_undoStack.emplace_back(std::move(cmd), description, estimatedMemory);
    m_currentMemoryUsage += estimatedMemory;

    // Trim stack to max depth
    while (m_undoStack.size() > m_maxUndoDepth) {
      m_currentMemoryUsage -= m_undoStack.front().estimatedMemory;
      m_undoStack.pop_front(); // Remove oldest (front)
    }

    // Trim stack to max memory
    while (m_currentMemoryUsage > m_maxMemoryBytes && !m_undoStack.empty()) {
      m_currentMemoryUsage -= m_undoStack.front().estimatedMemory;
      m_undoStack.pop_front();
    }

    // Clear redo history
    clearRedo();

    return true;
  }
  bool undo(Canvas &canvas) {
    if (m_undoStack.empty()) {
      Logger::log(LogLevel::DEBUG, "No undo available");
      return false;
    }

    CommandEntry entry = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    entry.command->undo(canvas);
    canvas.markDirty();

    m_redoStack.push_back(std::move(entry));
    m_currentMemoryUsage -= entry.estimatedMemory;

    Logger::log(LogLevel::DEBUG, "Undo: " + entry.description);
    return true;
  }

  bool redo(Canvas &canvas) {
    if (m_redoStack.empty()) {
      Logger::log(LogLevel::DEBUG, "No redo available");
      return false;
    }

    CommandEntry entry = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    entry.command->execute(canvas);
    canvas.markDirty();

    m_currentMemoryUsage += entry.estimatedMemory;
    m_undoStack.push_back(std::move(entry));

    Logger::log(LogLevel::DEBUG, "Redo: " + entry.description);
    return true;
  }

  // ====== QUERY OPERATIONS ======

  bool canUndo() const { return !m_undoStack.empty(); }
  bool canRedo() const { return !m_redoStack.empty(); }

  std::optional<std::string> getUndoDescription() const {
    if (m_undoStack.empty())
      return std::nullopt;
    return m_undoStack.back().description; // Most recent
  }

  std::optional<std::string> getRedoDescription() const {
    if (m_redoStack.empty())
      return std::nullopt;
    return m_redoStack.back().description;
  }

  size_t getUndoCount() const { return m_undoStack.size(); }
  size_t getRedoCount() const { return m_redoStack.size(); }
  size_t getCurrentMemoryUsage() const { return m_currentMemoryUsage; }
  size_t getMemoryLimit() const { return m_maxMemoryBytes; }

  float getMemoryUsagePercent() const {
    if (m_maxMemoryBytes == 0)
      return 0.0f;
    return (float)m_currentMemoryUsage / (float)m_maxMemoryBytes * 100.0f;
  }

  // ====== CONFIGURATION ======

  void setMaxUndoDepth(size_t depth) {
    m_maxUndoDepth = depth;
    trimUndoStack();
  }

  void setMaxMemoryMB(size_t mb) {
    m_maxMemoryBytes = mb * 1024 * 1024;
    trimUndoStack();
  }

  // ====== UTILITY ======

  void clear() {
    clearUndo();
    clearRedo();
  }

  void clearUndo() {
    while (!m_undoStack.empty()) {
      m_currentMemoryUsage -= m_undoStack.back().estimatedMemory;
      m_undoStack.pop_back();
    }
  }

  void clearRedo() {
    while (!m_redoStack.empty()) {
      m_redoStack.pop_back();
    }
  }

  std::string getDebugInfo() const {
    return std::format(
        "Undo: {} | Redo: {} | Memory: {}KB / {}MB ({}%)", m_undoStack.size(),
        m_redoStack.size(), m_currentMemoryUsage / 1024,
        m_maxMemoryBytes / 1024 / 1024, (int)getMemoryUsagePercent());
  }

private:
  void trimUndoStack() {
    // Trim oldest entries first
    while ((m_undoStack.size() > m_maxUndoDepth ||
            m_currentMemoryUsage > m_maxMemoryBytes) &&
           !m_undoStack.empty()) {
      m_currentMemoryUsage -= m_undoStack.front().estimatedMemory;
      m_undoStack.pop_front();
    }
  }
};
