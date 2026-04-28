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

TODO: refinements;
Based on your code and project structure, you have a solid foundation using the **Command** and **Strategy** patterns. However, there are architectural "chokepoints" and memory management risks that will make the project harder to scale as you add more tools or features like layers.

Here is a breakdown of what you can refine to improve modularity and efficiency:

### 1. Fix the Memory Management (Immediate Refinement)

Currently, your `Pencil`, `Eraser`, and `Line` tools all manage their own `SDL_Surface* currentSnapshot`. This is high-risk because:

- **Dangling Pointers:** If you switch tools while `drawing == true`, the snapshot might never be freed or might be accessed by the wrong tool.
- **Code Duplication:** Every tool repeats the exact same snapshot logic.

**Refinement:** Move snapshot logic into `BaseTool` or a `Snapshot` helper class.

```cpp
// In BaseTool.h
virtual void cleanup() {
    if (currentSnapshot) {
        SDL_DestroySurface(currentSnapshot);
        currentSnapshot = nullptr;
    }
}
```

Ensure your `ToolManager::setActiveTool` calls `cleanup()` on the previous tool before switching.

### 2. Decouple Drawing Algorithms from Tools (Modularity)

Right now, `Pencil`, `Eraser`, and `Line` all have their own `bresenham` and `dda` implementations. If you find a bug in Bresenham, you have to fix it in three places.

**Refinement:** Create a `Geometry` or `Algorithms` namespace.

- **Why:** Tools should only handle _input logic_ (mouse clicks). The _math_ of how to turn two points into pixels should be central.
- **Easier to add:** If you want to add a "Circle" tool later, you just call `Algorithms::drawCircle` instead of rewriting pixel loops.

### 3. Move the "Brush Block" to a Single Function (Efficiency)

In every tool, you have an "Optimized Brush Block" that loops through `oy` and `ox` to draw thick lines.

**Refinement:** Create a `Canvas::putPixel(int x, int y, int size, uint32_t color)` method.

- **Efficiency:** You can optimize this one function (using `memset` for rows or SIMD) and every tool in your app instantly gets faster.

### 4. Improve Command Granularity (Scaling)

Your `DrawCommand` stores two full `SDL_Surface` snapshots for every single stroke.

- **The Problem:** If your canvas is 4K, an undo stack of 50 commands will consume gigabytes of RAM.
- **Refinement:** \* **Dirty Rectangles:** Only snapshot the bounding box of where the user actually drew.
  - **Action Recording:** Instead of snapshots, store the path (list of points). To "Undo," clear the canvas and re-render all commands except the last one.

### 5. Future-Proofing: The Layer System

If you want to add layers later, your `App` and `Canvas` are currently too tightly coupled to a single `drawingSurface`.

**Refinement:**

- Change `Canvas` to hold a `std::vector<Layer>`.
- A `Layer` would contain the `SDL_Surface`.
- `Canvas::syncTexture` would then composite (blend) all layers onto the `mainTexture`.

### Summary of Action Items:

1.  **Centralize Math:** Move `bresenham` and `dda` out of the tool classes into a static utility file.
2.  **Tool Destructors:** Add proper destructors to all tools to `SDL_DestroySurface(currentSnapshot)` to prevent the leaks/crashes you are seeing.
3.  **Use Globals Wisely:** Use `g_CurrentAlgo` in the centralized math file so you don't have to pass it into every tool function.
