#include <map>
#include <memory>
#include <string>

#include "BaseTool.h"

#include "Systems/Logger.h"

class ToolManager {
  std::map<std::string, std::unique_ptr<BaseTool>> registry;
  BaseTool *activeTool = nullptr;

public:
  void registerTool(std::string name, std::unique_ptr<BaseTool> tool) {
    registry[name] = std::move(tool); // "Move" ownership into the map
    std::string message = "TOOL REGISTERED:" + name;
    Logger::log(LogLevel::INFO, message);
  }

  void setActiveTool(std::string name) {
    if (registry.count(name)) {
      if (activeTool) {
        activeTool->deactivate();
      }
      activeTool = registry[name].get();
      std::string message = "THE ACTIVE TOOL:" + name;
      Logger::log(LogLevel::INFO, message);
    }
  }
  void reset() {
    if (activeTool) {
      activeTool->deactivate();
      //     activeTool = nullptr;
    }
    Logger::debug("ToolManager reset");
  }
  BaseTool *getActive() { return activeTool; }
};
