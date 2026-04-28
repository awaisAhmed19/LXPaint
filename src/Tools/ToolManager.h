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
  }

  void setActiveTool(std::string name) {
    if (registry.count(name)) {
      if (activeTool) {
        activeTool->deactivate();
      }
      activeTool = registry[name].get();
    }
  }
  BaseTool *getActive() { return activeTool; }
};
