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

## DONE

[X] DrawCommand snapshot timing
[X] canvas.markDirty() undo/execute
[X] screenToCanvas transform correctness
[X] Bresenham steep transpose fix
[X] Pencil raster consistency
[X] UI click-through/input passthrough
[X] Interaction ownership violations
[X] Snapshot corruption bounds
[X] Null/error guards
[X] Preview clearing
[X] Surface locking correctness
[X] Undo stack limits
[X] DrawCommand lifecycle leaks
[X] Preview zoom/pan mismatch
[X] Texture recreation on resize
[X] Canvas coordinate clamping
[X] RenderTarget::clearRGBA()
[X] Preview transparent / Canvas opaque init
[X] Modal tool lifecycle
[X] Freehand tool lifecycle
[X] Logger label cleanup
[X] Snapshot efficiency improvements
[X] Redundant interaction toggles
[X] Remove dead renderer lifecycle
[X] Viewport-centered rendering
[X] Pan/zoom ownership system

========================
PHASE 1 — CORE PRODUCT COMPLETION
========================

[X] Finalize Canvas::resize() architecture
-> ensure viewport, texture lifecycle, preview, and future layers remain coherent

[ ] Add save/load support
-> PNG export/import pipeline
-> document persistence foundation

[ ] Implement flood fill tool
-> proper bounds handling
-> stack-safe traversal
-> undo integration

[ ] Add layer system foundation
-> layer abstraction
-> active layer ownership
-> renderer composition pipeline

========================
PHASE 2 — STABILITY HARDENING
========================

[ ] Centralize dirty invalidation near raster mutation source

[ ] Fix inconsistent line/rect snapshot padding causing ghost pixels

[ ] Fix drawRect always filling white internally

[ ] Refactor duplicated bounds calculation logic

========================
PHASE 3 — PERFORMANCE
========================

[ ] Implement dirty-region texture uploads in Renderer::sync()

[ ] Add thread safety to Profiler static containers

[ ] Fully resolve Logger history race conditions

========================
PHASE 4 — ARCHITECTURE CLEANUP
========================

[ ] Separate color semantics from pixel storage representation

[ ] Replace hardcoded packed color constants with SDL_MapRGBA()

[ ] Reduce Globals.h header pollution

[ ] Remove mixed SDL_Log / Logger inconsistency

[ ] Add const correctness across rendering/tool APIs

========================
PHASE 5 — FUTURE SYSTEMS
========================

[ ] Add proper anti-aliased line rendering

[ ] Add rasterizer unit tests

[ ] Add fullscreen escape handling

[ ] Handle high-DPI scaling correctly

[ ] Add tablet/pressure support

[ ] Clean up profiler state persistence between sessions
