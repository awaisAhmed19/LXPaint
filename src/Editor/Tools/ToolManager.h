#pragma once

#include <map>
#include <memory>
#include <string>

#include "BaseTool.h"
#include "ClickTool.h"
#include "Systems/Logger.h"

class ToolManager {
private:
  std::map<std::string, std::unique_ptr<BaseTool>> m_registry;
  std::map<std::string, std::unique_ptr<ClickTool>> m_clickRegistry;

  BaseTool *m_activeTool = nullptr;
  ClickTool *m_activeClickTool = nullptr;

  ToolType m_activeType = ToolType::Pencil;

public:
  void registerTool(const std::string &name, std::unique_ptr<BaseTool> tool) {
    m_registry[name] = std::move(tool);
    Logger::log(LogLevel::INFO, "TOOL REGISTERED: " + name);
  }

  void registerTool(const std::string &name, std::unique_ptr<ClickTool> tool) {
    m_clickRegistry[name] = std::move(tool);
    Logger::log(LogLevel::INFO, "TOOL REGISTERED: " + name);
  }

  void setActiveTool(const std::string &name, ToolType type) {
    if (m_activeTool) {
      m_activeTool->deactivate();
      m_activeTool = nullptr;
    }

    if (m_activeClickTool) {
      m_activeClickTool->deactivate();
      m_activeClickTool = nullptr;
    }

    m_activeType = type;

    if (auto it = m_registry.find(name); it != m_registry.end()) {
      m_activeTool = it->second.get();
    } else if (auto cit = m_clickRegistry.find(name);
               cit != m_clickRegistry.end()) {
      m_activeClickTool = cit->second.get();
    }

    Logger::log(LogLevel::INFO, "ACTIVE TOOL: " + name);
  }

  void reset() {
    if (m_activeTool)
      m_activeTool->deactivate();

    if (m_activeClickTool)
      m_activeClickTool->deactivate();

    Logger::debug("ToolManager reset");
  }

  BaseTool *getActiveTool() { return m_activeTool; }
  ClickTool *getActiveClickTool() { return m_activeClickTool; }
  ToolType getActiveToolType() const { return m_activeType; }
};
