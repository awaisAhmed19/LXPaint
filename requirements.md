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

LXPAINT MASTER TODO / BUG AUDIT

========================
CRITICAL
========================

[X] Fix DrawCommand snapshot timing (capture BEFORE on mouse down, AFTER on mouse up)

[X] Add canvas.markDirty() inside DrawCommand::undo() and execute()

[ ] Fix coordinate transform pipeline: ALL tools must use renderer.screenToCanvas()

[ ] Unify coordinate spaces (screen space vs canvas space vs viewport space)

[ ] Fix Bresenham steep-line transpose logic (missing coordinate swapping)

[ ] Fix Pencil continuous rasterization/presentation inconsistencies

[ ] Fix UI input passthrough / click-through interaction issues

[ ] Prevent tools from directly mutating interaction state ownership

[ ] Fix potential undo/redo snapshot corruption outside dirty region bounds

========================
HIGH
========================

[ ] Replace SDL_TEXTUREACCESS_STREAMING with SDL_TEXTUREACCESS_STATIC

[ ] Add null/error guards for texture creation, upload, and render paths

[ ] Ensure preview layer clears correctly after modal commits

[ ] Fix thick-line rasterization ("sausage" thickness inconsistency)

[ ] Fix inconsistent surface locking across rasterizer paths

[ ] Fix Renderer::sync() updating entire texture every frame instead of dirty regions

[ ] Add maximum undo/redo stack limit to prevent OOM crashes

[ ] Fix DrawCommand memory lifecycle leaks in modal tools

[ ] Fix coordinate mismatch for preview rendering with zoom/pan

[ ] Add proper texture recreation handling on canvas resize

[ ] Fix incomplete SDL surface locking in dda/rectFill paths

[ ] Fix race condition in Logger history access

[ ] Add thread safety to Profiler static containers

[ ] Add proper canvas coordinate clamping during zoom/pan

========================
MEDIUM
========================

[ ] Replace hardcoded packed color constants with SDL_MapRGBA()

[ ] Separate color semantics from pixel storage representation

[ ] Add RenderTarget::clearRGBA() abstraction

[ ] Make PreviewLayer initialize transparent and Canvas initialize opaque white

[ ] Centralize dirty invalidation closer to raster mutation source

[ ] Formalize modal tool lifecycle (begin/update/commit/cancel/clear)

[ ] Formalize freehand tool lifecycle separately from modal tools

[ ] Fix drawRect always filling white internally

[ ] Remove stale/copy-pasted logger labels

[ ] Refactor duplicated bounds calculation logic

[ ] Reduce Globals.h header pollution

[ ] Remove mixed SDL_Log and custom Logger inconsistency

[ ] Add const correctness across rendering/tool APIs

[ ] Improve DrawCommand snapshot efficiency for tiny dirty regions

[ ] Fix inconsistent line/rect snapshot padding causing ghost pixels

[ ] Remove redundant manual interaction state toggles in tools

========================
LOW
========================

[ ] Remove dead Renderer::begin()/end() lifecycle code completely

[ ] Add Canvas::resize() support

[ ] Add viewport-centered canvas rendering

[ ] Add proper viewport/pan/zoom ownership system

[ ] Add layer system foundation

[ ] Add save/load support

[ ] Implement flood fill tool

[ ] Add fullscreen escape handling

[ ] Add rasterizer unit tests

[ ] Add proper anti-aliased line rendering

[ ] Remove unused DDA benchmarking dead paths

[ ] Add tablet/pressure support later

[ ] Handle high-DPI scaling correctly

[ ] Clean up profiler state persistence between sessions
