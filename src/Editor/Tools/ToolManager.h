#include <map>
#include <memory>
#include <string>

#include "BaseTool.h"

#include "ClickTool.h"
#include "Systems/Logger.h"

class ToolManager {
  std::map<std::string, std::unique_ptr<BaseTool>> registry;
  std::map<std::string, std::unique_ptr<ClickTool>> clickRegistry;
  BaseTool *activeTool = nullptr;
  ClickTool *activeClickTool = nullptr;

public:
  void registerTool(std::string name, std::unique_ptr<ClickTool> tool) {
    clickRegistry[name] = std::move(tool); // "Move" ownership into the map
    std::string message = "TOOL REGISTERED:" + name;
    Logger::log(LogLevel::INFO, message);
  }
  void registerTool(std::string name, std::unique_ptr<BaseTool> tool) {
    registry[name] = std::move(tool); // "Move" ownership into the map
    std::string message = "TOOL REGISTERED:" + name;
    Logger::log(LogLevel::INFO, message);
  }
  void setActiveTool(const std::string &name) {
    if (activeTool) {
      activeTool->deactivate();
      activeTool = nullptr;
    }

    if (activeClickTool) {
      activeClickTool->deactivate();
      activeClickTool = nullptr;
    }

    if (auto it = registry.find(name); it != registry.end()) {
      activeTool = it->second.get();
      Logger::log(LogLevel::INFO, "THE ACTIVE TOOL: " + name);
    } else if (auto it = clickRegistry.find(name); it != clickRegistry.end()) {
      activeClickTool = it->second.get();
      Logger::log(LogLevel::INFO, "THE ACTIVE TOOL: " + name);
    }
  }
  void reset() {
    if (activeTool) {
      activeTool->deactivate();
      //     activeTool = nullptr;
    }

    Logger::debug("ToolManager reset");
  }
  BaseTool *getActiveTool() { return activeTool; }
  ClickTool *getActiveClickTool() { return activeClickTool; }
};
