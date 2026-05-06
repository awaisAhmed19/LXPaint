LXPaint
│
├── App ← owns the game loop, ties everything together
│ ├── InputHandler ← translates raw mouse/keyboard into events
│ ├── StateManager ← what tool is active, current color, brush size
│ └── Renderer ← draws canvas + UI each frame
│
├── Canvas ← owns the RenderTexture2D
│ ├── applyCommand(Command\*) ← executes a draw operation
│ ├── undo() ← pops undo stack
│ └── redo() ← pops redo stack
│
├── Tools ← each tool produces Commands
│ ├── PencilTool ← produces DrawStrokeCommand
│ ├── FillTool ← produces FloodFillCommand
│ ├── ShapeTool ← produces DrawShapeCommand
│ └── EraserTool ← produces DrawStrokeCommand (color=WHITE)
│
├── Commands ← the actions themselves
│ ├── DrawStrokeCommand
│ ├── FloodFillCommand
│ └── DrawShapeCommand
│
└── UI ← toolbar, palette, brush size slider
├── Toolbar
├── ColorPalette
└── BrushSizeSlider

# LXPaint Session Context Blob

## Project Overview

User is building a low-level MS Paint–style application called LXPaint using:

* C++20
* SDL3
* ImGui
* CPU-side pixel rendering
* Custom line algorithms (Bresenham + DDA)
* Command-based undo/redo system
* Dirty rectangle optimization
* Profiling/benchmark visualization tools

User wants deep architectural understanding, not just working code.

---

# Current Architecture

## Rendering Pipeline

Canvas contains a 4-layer architecture:

1. CPU Layer

```cpp
SDL_Surface* drawingSurface;
```

* Source of truth
* Pixel writes happen here
* Bresenham/DDA modify this directly

2. Sync Bridge

```cpp
syncTexture()
```

* Uses SDL_UpdateTexture
* Copies CPU surface -> GPU texture
* Current bottleneck: full-surface sync every frame

3. GPU Main Layer

```cpp
SDL_Texture* mainTexture;
```

* Display texture shown on screen
* Updated from drawingSurface

4. GPU Preview Layer

```cpp
SDL_Texture* previewTexture;
```

* Transparent overlay for temporary previews
* Intended for line/rect preview
* Better than XOR or snapshot restore

Rendering flow:

User Input
-> Tool
-> Renderer
-> drawingSurface
-> syncTexture()
-> mainTexture

* previewTexture
  -> SDL_RenderPresent

---

# App::render()

Current render loop:

```cpp
void App::render() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
  SDL_RenderClear(renderer);

  canvas->syncTexture();

  SDL_FRect dest = {0,0,(float)canvas->w,(float)canvas->h};

  SDL_RenderTexture(renderer, canvas->mainTexture, NULL, &dest);
  SDL_RenderTexture(renderer, canvas->previewTexture, NULL, &dest);

  DrawLogConsole(*canvas, screenW, screenH,
                 frameTimes, frameOffset);

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
}
```

Important insight:

* previewTexture MUST be rendered after mainTexture
* otherwise GPU previews are invisible

---

# Renderer System

## Algorithms Implemented

### DDA

* Float increment line algorithm
* Uses xInc/yInc stepping
* Thick lines originally implemented via square brush loop

### Bresenham

* Integer-only incremental line algorithm
* Optimized into span-based thick rendering

---

# Thick Line Rendering

User implemented span-based rendering.

## Vertical Span

```cpp
void drawVerticalSpan(...)
```

* Draws vertical thickness column
* Used for shallow slopes

## Horizontal Span

```cpp
void drawHorizontalSpan(...)
```

* Draws horizontal thickness row
* Used for steep slopes

Concept:

* Instead of stamping square brush each pixel
* Renderer stretches spans perpendicular to line direction

This creates:

* cleaner thick lines
* fewer memory writes
* better cache behavior

---

# XOR Preview Experiment

User experimented with XOR preview:

```cpp
px ^= (color & 0x00FFFFFF);
```

Insights discovered:

* XOR cancels itself if same pixels drawn twice
* Works poorly for thick brushes
* Snapshot restore + XOR conflict conceptually
* Good for old-school debug overlays
* Bad long-term preview solution

Conclusion:

* previewTexture is superior

---

# Line Tool Evolution

## Original Problem

Line tool produced:

* ghost trails
* jitter
* disappearing lines
* spaghetti artifacts

Causes:

* XOR cancellation
* restoring incorrect dirty rect
* growing bounding boxes
* mixing snapshot restore + XOR

---

## Correct CPU Rubber Band Solution

User implemented:

* currentSnapshot
* prevBound
* computeLineBounds()
* restore previous segment only

Flow:

1. onMouseDown

* duplicate snapshot
* initialize prevBound

2. onMouseMove

* restore prevBound region
* compute new bounds
* draw preview line
* update prevBound

3. onMouseUp

* restore preview
* draw final line
* create DrawCommand

Important insight:

* previewBounds and strokeBounds should be separate

---

# Rectangle Tool

Initial bug:

* user accidentally drew diagonal line instead of rectangle

Correct solution:

* rectangle = 4 Bresenham lines

drawRect() implemented using:

* top edge
* bottom edge
* left edge
* right edge

Rect tool mirrors line tool architecture:

* snapshot restore
* preview bounds
* final commit

---

# Command System

## Command Base

```cpp
class Command {
public:
  virtual void execute(Canvas&) = 0;
  virtual void undo(Canvas&) = 0;
};
```

Concept:

* Command pattern
* execute() = redo
* undo() = revert

---

## CommandManager

Maintains:

```cpp
std::stack<Command*> undoStack;
std::stack<Command*> redoStack;
```

Flow:

* executeCommand()
* undo()
* redo()

---

# Dirty Rectangle Undo System

## DrawCommand

Stores:

```cpp
SDL_Rect region;
UniqueSurface before;
UniqueSurface after;
```

Purpose:

* store only changed region
* not full canvas snapshot

Flow:

Constructor:

* captures BEFORE snapshot

captureAfter():

* captures AFTER snapshot

execute():

* blits AFTER back to canvas

undo():

* blits BEFORE back to canvas

Important insight:

* DrawCommand acts like a pixel time machine

---

# UniqueSurface Design

```cpp
using UniqueSurface = std::unique_ptr<SDL_Surface, SDL_Surface_Deleter>;
```

Custom deleter automatically calls:

```cpp
SDL_DestroySurface()
```

Benefits:

* RAII
* no leaks
* automatic cleanup

---

# Profiler System

Profiler tracks:

* live timings
* race comparisons
* frame history
* distance traveled

Structures:

```cpp
AlgoRun
AlgoStats
RaceResult
```

Current profiling verdict:

GOOD:

* interactive telemetry
* optimization feedback
* visual benchmarking

BAD:

* not scientifically fair
* memory writes dominate timing
* cache effects bias results
* mouse movement changes workload

Conclusion:

* current profiler is a live telemetry system
* not a rigorous benchmark suite

Recommended future improvement:

* isolated surfaces
* fixed test cases
* repeated runs
* benchmark-only mode

---

# BaseTool Architecture

Current BaseTool:

```cpp
class BaseTool
```

Contains:

* currentSnapshot
* Boundbox
* updateBounds()
* resetBounds()

Insights:

GOOD:

* abstract tool interface
* centralized shared logic
* tool lifecycle abstraction

BAD:

* BaseTool knows too much about rendering strategy
* snapshot system tightly coupled
* Boundbox overloaded with multiple responsibilities

Recommended future evolution:

Split bounds into:

* previewBounds
* strokeBounds

Move preview logic out of BaseTool.

Desired architecture:

Tool = WHAT
Renderer = HOW
Canvas = WHERE

---

# CMake Lessons Learned

Original issue:

```cmake
file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")
```

Problem:

* zero compilation control

Improved solution:

```cmake
set(PROJECT_SOURCES
    src/main.cpp
    src/App.cpp
    src/Core/Renderer.cpp
    src/Tools/Pencil.cpp
)
```

Future direction:

* optional tool toggles
* modular compilation

---

# UI / Console Lessons

Issues encountered:

* circular includes
* multiple definition linker errors

Key lessons:

1. Headers declare
2. CPP files define
3. Avoid UI depending directly on App internals
4. Pass data into UI functions instead

---

# Current Technical Direction

Immediate recommended next step:

Convert Line + Rect tools fully to previewTexture.

Reason:

* removes snapshot restore complexity
* removes XOR hacks
* GPU accelerated previews
* cleaner architecture

Future roadmap:

1. GPU preview pipeline
2. Dirty-region texture syncing
3. ToolContext abstraction
4. Layer system
5. Zoom/pan transforms
6. Full GPU rendering pipeline
7. Path-based undo system

---

# Core Insights Discovered During Session

1. Memory bandwidth often dominates algorithm math.
2. Dirty rectangles are harder than they initially appear.
3. XOR preview is historically interesting but architecturally inferior.
4. GPU preview layers solve many CPU preview problems.
5. Tools should express intent, not own rendering details.
6. Rendering architecture matters more than algorithms at this stage.
7. Undo systems are essentially pixel time machines.
8. CPU surface = truth, GPU textures = projection.

---

# Recommended Continuation Prompt

Paste this into a future session:

"Continue helping me evolve my LXPaint architecture. Read the context blob carefully first. Focus on converting Line/Rect tools to previewTexture-based GPU previews and improving separation between Tool, Renderer, and Canvas responsibilities."
