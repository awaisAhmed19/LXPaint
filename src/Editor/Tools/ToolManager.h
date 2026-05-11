#include "../Systems/Logger.h"
#include "BaseTool.h"
#include <map>
#include <memory>
#include <string>
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
  BaseTool *getActive() { return activeTool; }
};
