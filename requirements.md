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
========================
CRITICAL
========================

[X] Fix DrawCommand snapshot timing (capture BEFORE on mouse down, AFTER on mouse up)
[X] Add canvas.markDirty() inside DrawCommand::undo() and execute()

[ ] Fix coordinate transform pipeline: ALL tools must use renderer.screenToCanvas()

[ ] Unify coordinate spaces (screen space vs canvas space vs viewport space)

[X] Fix Bresenham steep-line transpose logic (missing coordinate swapping)

[X] Fix Pencil continuous rasterization/presentation inconsistencies

[ ] Fix UI input passthrough / click-through interaction issues

[X] Prevent tools from directly mutating interaction state ownership

[X] Fix potential undo/redo snapshot corruption outside dirty region bounds

========================
HIGH
========================

[ ] Replace SDL_TEXTUREACCESS_STREAMING with SDL_TEXTUREACCESS_STATIC
-> actually opposite probably needed eventually for frequent uploads

[X] Add null/error guards for texture creation, upload, and render paths

[X] Ensure preview layer clears correctly after modal commits
-> effectively solved via conditional preview rendering

[ ] Fix thick-line rasterization ("sausage" thickness inconsistency)

[ ] Fix inconsistent surface locking across rasterizer paths

[ ] Fix Renderer::sync() updating entire texture every frame instead of dirty regions

[ ] Add maximum undo/redo stack limit to prevent OOM crashes

[X] Fix DrawCommand memory lifecycle leaks in modal tools

[ ] Fix coordinate mismatch for preview rendering with zoom/pan

[ ] Add proper texture recreation handling on canvas resize

[ ] Fix incomplete SDL surface locking in dda/rectFill paths

[ ] Fix race condition in Logger history access
-> partially solved with mutex, but erase/history iteration may still race

[ ] Add thread safety to Profiler static containers

[ ] Add proper canvas coordinate clamping during zoom/pan

========================
MEDIUM
========================

[ ] Replace hardcoded packed color constants with SDL_MapRGBA()

[ ] Separate color semantics from pixel storage representation

[X] Add RenderTarget::clearRGBA() abstraction

[X] Make PreviewLayer initialize transparent and Canvas initialize opaque white
-> mostly solved assuming your preview clearRGBA(0,0,0,0) path is active

[ ] Centralize dirty invalidation closer to raster mutation source

[X] Formalize modal tool lifecycle (begin/update/commit/cancel/clear)

[X] Formalize freehand tool lifecycle separately from modal tools

[ ] Fix drawRect always filling white internally

[X] Remove stale/copy-pasted logger labels

[ ] Refactor duplicated bounds calculation logic

[ ] Reduce Globals.h header pollution

[ ] Remove mixed SDL_Log and custom Logger inconsistency

[ ] Add const correctness across rendering/tool APIs

[X] Improve DrawCommand snapshot efficiency for tiny dirty regions
-> much better now with region-based stroke snapshots

[ ] Fix inconsistent line/rect snapshot padding causing ghost pixels

[X] Remove redundant manual interaction state toggles in tools

========================
LOW
========================

[X] Remove dead Renderer::begin()/end() lifecycle code completely
-> effectively dead/unused now

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
