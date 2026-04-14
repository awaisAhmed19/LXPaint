#include "BaseTool.h"
#include <map>
#include <string>
class ToolManager {
  std::map<std::string, BaseTool *> registry;
  BaseTool *activeTool = nullptr;

public:
  void registerTool(std::string name, BaseTool *tool) { registry[name] = tool; }

  void setActiveTool(std::string name) {
    if (registry.count(name))
      activeTool = registry[name];
  }

  BaseTool *getActive() { return activeTool; }
};
