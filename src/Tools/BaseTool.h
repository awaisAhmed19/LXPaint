#pragma once
#include "../Globals.h"
#include "../core/Command.h"
class Canvas;
class BaseTool {
   public:
    virtual ~BaseTool() {}
    virtual void onMouseDown(vec2<float> pos, Canvas& canvas) = 0;
    virtual void onMouseMove(vec2<float> pos, Canvas& canvas) = 0;
    virtual Command* onMouseUp(vec2<float> pos, Canvas& canvas) = 0;
};
