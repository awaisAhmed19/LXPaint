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
IMPORTANT THING YOU WILL HIT NEXT

Your bounds calculation probably sucks right now.

Meaning:

snapshot region too small

which causes clipping during undo/redo.

So VERY soon you’ll want:

expandBoundsByBrushSize()

Otherwise edges vanish during restore.

THE BIG WIN HERE

Now:

Pencil
Line
Rect
Eraser
Bucket
Spray
Future brush engine

ALL use SAME history system.

That’s the important part.

The engine becomes coherent instead of tool-specific spaghetti tentacles fighting in the dark at 2 AM.
LXPAINT MASTER TODO / BUG AUDIT

# REAL DEADLINE TODO

## P0 — WILL BREAK THE APP / DEMO

### Undo / Redo

- [ ] Fix Eraser undo lifecycle
  - [ ] Create command on `mouseDown`
  - [ ] Finalize on `mouseUp`

- [ ] Verify ALL tools follow:

  ```text id="zmu4cx"
  mouseDown -> begin command
  mouseMove -> draw/preview
  mouseUp   -> finalize command
  ```

- [ ] Fix potential undo corruption outside dirty bounds
- [ ] Ensure undo clears redo stack correctly
- [ ] Ensure empty undo/redo cannot crash

---

### Coordinate System (VERY IMPORTANT)

- [ ] Make ALL tools use:

  ```cpp id="gvijba"
  renderer.screenToCanvas()
  ```

- [ ] Fix mixed coordinate spaces
  - screen
  - canvas
  - viewport
  - zoom/pan

- [ ] Fix preview rendering mismatch during zoom/pan
- [ ] Add coordinate clamping during zoom/pan

If this remains broken:

```text id="c2rm2s"
tools will draw at wrong positions under zoom/pan
```

---

### Rendering / Visual Bugs

- [ ] Fix canvas flickering
- [ ] Prevent drawing under ImGui windows
- [ ] Fix transparent/invisible canvas issue
- [ ] Ensure preview layer clears after commit
- [ ] Verify zoom/pan still renders correctly

---

### Rasterization Bugs

- [ ] Fix Bresenham steep-line transpose logic
- [ ] Fix thick-line inconsistency
- [ ] Fix ghost pixels from snapshot padding

---

### Stability / Crash Safety

- [ ] Add null guards for:
  - texture creation
  - texture upload
  - rendering
  - surfaces

- [ ] Fix inconsistent SDL surface locking
- [ ] Fix incomplete locking in DDA / rectFill
- [ ] Add undo stack size limit

---

# P1 — PERFORMANCE STABILIZATION

### Texture Uploading

- [ ] Replace:

  ```cpp id="jjfrn0"
  SDL_TEXTUREACCESS_STREAMING
  ```

  with:

  ```cpp id="7rqlwo"
  SDL_TEXTUREACCESS_STATIC
  ```

### Dirty Updates

- [ ] Stop syncing full texture every frame
- [ ] Use dirty-region uploads only

### Hotpath Cleanup

- [ ] Remove hotpath SDL_Log spam
- [ ] Remove profiler spam
- [ ] Remove dead benchmarking paths

---

# P2 — QUICK CLEANUP

### Interaction Ownership

- [ ] Remove tools directly mutating interaction state
- [ ] Remove redundant interaction toggles

### Logging Cleanup

- [ ] Remove stale/copy-pasted logger labels
- [ ] Stop mixing SDL_Log and Logger

### Build Cleanup

- [ ] Remove debug-only code
- [ ] Remove commented dead code
- [ ] Final release build test

---

# IGNORE UNTIL AFTER DEADLINE

Do NOT touch:

- layer system
- save/load
- flood fill
- anti-aliasing
- ECS
- renderer rewrite
- serialization
- operation graph
- viewport ownership redesign
- perfect architecture cleanup
- unit tests
- pressure support

Those are post-submission problems.

Right now your battlefield priorities are:

```text id="e3j10y"
1. Undo works
2. Coordinates work
3. Rendering stable
4. No crashes
5. Ship
```

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
