# LXPaint — Roadmap

---

## Table of Contents

1. [Verified Issues — Fix First](#verified-issues--fix-first)
2. [Partially Verified — Investigate Before Acting](#partially-verified--investigate-before-acting)
3. [Future Work — Not Yet Implemented](#future-work--not-yet-implemented)
4. [Improvements.md Verification Status](#improvementsmd-verification-status)

---

## Verified Issues — Fix First

These are confirmed by source code evidence. See `AUDIT.md` for full detail.

---

### Fix SDL3 lockSurface

**Source:** `src/App/Globals.h::lockSurface()`  
**AUDIT ref:** AUDIT-001

```cpp
// Current (wrong in SDL3)
if (SDL_LockSurface(surface) < 0) return false;

// Correct
if (!SDL_LockSurface(surface)) return false;
```

`SDL_LockSurface` returns `bool` in SDL3. The `< 0` check never evaluates to
true on a bool. Lock failures are silently swallowed.

**Files to change:** `src/App/Globals.h`

---

### Implement or Remove floodFill

**Source:** `src/Rendering/Rasterizer.h`, `src/Rendering/Rasterizer.cpp`  
**AUDIT ref:** AUDIT-002, AUDIT-003

`floodFill` is declared but has no definition. `floodFillHelper` exists but
contains four logic bugs (wrong dimension variables, wrong recursive
directions, no overflow protection).

**Option A:** Remove both the declaration and `floodFillHelper` until the fill
tool is ready.

**Option B:** Implement `floodFill` with a correct iterative (stack-based)
algorithm and remove `floodFillHelper`.

**Files to change:** `src/Rendering/Rasterizer.h`, `src/Rendering/Rasterizer.cpp`

---

### Reconcile drawRectFill Signature

**Source:** `src/Rendering/Rasterizer.h`, `src/Rendering/Rasterizer.cpp`  
**AUDIT ref:** AUDIT-004

Header declares `int brushSize` parameter that the implementation does not
have. Align the header with the implementation (remove `brushSize` from the
header) or add it to the implementation.

**Files to change:** `src/Rendering/Rasterizer.h` (or `.cpp`)

---

### Fix SnapshotCommand Dirty Rect Propagation

**Source:** `src/Editor/Commands/SnapshotCommand.h`  
**AUDIT ref:** AUDIT-005

`undo()` and `redo()` call `canvas.markDirty()`, invalidating the full
canvas. `m_bounds` is available and should be used with `invalidateRect`.

```cpp
// Current
canvas.markDirty();

// Correct
canvas.invalidateRect(m_bounds);
```

**Files to change:** `src/Editor/Commands/SnapshotCommand.h`

---

### Notify Viewport After Canvas Resize

**Source:** `src/Editor/Editor.cpp::resizeCanvas()`  
**AUDIT ref:** AUDIT-009

After `m_canvas.resize(w, h, policy)`, the viewport's stored canvas
dimensions are stale. `isPointInCanvas` and `clampPan` use wrong values.

```cpp
// Add after m_canvas.resize():
m_viewport.onCanvasResized(m_canvas.getWidth(), m_canvas.getHeight());
```

**Files to change:** `src/Editor/Editor.cpp`

---

### Handle Window Resize Events

**Source:** `src/Editor/Editor.cpp` constructor, `src/App/App.cpp`  
**AUDIT ref:** AUDIT-010

Viewport screen rect is hardcoded to `{0, 0, 1280, 720}`. Window is created
`SDL_WINDOW_RESIZABLE`. No resize event is handled.

Add `SDL_EVENT_WINDOW_RESIZED` handling in `App::Application::handleEvents()`
and propagate the new size to the viewport.

**Files to change:** `src/App/App.cpp`, `src/Editor/Editor.cpp` (or `Editor.h`
to expose a `onWindowResized` method)

---

## Dead Code Removal

These are all low-risk, verified removals. Each takes minutes.

| What | File | AUDIT ref |
|---|---|---|
| `Canvas::m_preview` (unused PreviewLayer) | `Canvas.h`, `Canvas.cpp` | AUDIT-006 |
| `RenderTarget::m_boundingBox` (write-only) | `RenderTarget.h`, tool files | AUDIT-007 |
| `RenderTarget::updateBounds()` (no call sites) | `RenderTarget.h`, `RenderTarget.cpp` | AUDIT-007 |
| `Canvas::m_renderTargets` (never populated) | `Canvas.h` | AUDIT-008 |
| `float deltaTime` in `App::run()` | `App.cpp` | AUDIT-011 |
| `handleMouseDown/Move/Up` declarations | `Editor.h` | AUDIT-012 |
| `ToolInteractionState::modifier` | `ToolInteractionState.h` | AUDIT-013 |
| `ToolInteractionState::interactionID` | `ToolInteractionState.h` | AUDIT-013 |
| `int dx, dy` in `Circle::onMouseMove/Up` | `Circle.cpp` | AUDIT-014 |
| `Renderer::begin()` and `end()` declarations | `Renderer.h` | AUDIT-016 |

**Order:** Remove `Canvas::m_preview` last — confirm Canvas is no longer
referred to for preview by any new code first.

---

## Partially Verified — Investigate Before Acting

These are plausible issues that require more investigation before any change.

---

### InputCommand::FILL Has No Binding

**Source:** `src/Input/InputDispatcher.h`

`InputCommand::FILL` exists in the enum. `Editor::setupInputBindings()` does
not bind it. No fill tool exists in `ToolManager`. This is consistent with
`floodFill` not being implemented. Confirm by checking if FILL has any binding
or if a fill tool is registered elsewhere before adding one.

**Status:** Consistent with AUDIT-002 but requires a decision: implement fill
or remove FILL from the enum.

---

### Profiler Data Is Wired but May Be Unpopulated

**Source:** `src/Systems/Profiler.h`, `src/UI/Console.h`

`Profiler::algoData` and `Profiler::comparisonStorage` are used by ImGui
panels in `Console.h`. `PROFILE_FUNCTION()` macro exists but no call sites
were found in the provided tool or renderer code. The panels would render but
display empty data.

**Investigation needed:** Search for `PROFILE_FUNCTION()` and
`Profiler::recordRaceStep()` call sites beyond the provided files.

---

## Future Work — Not Yet Implemented

Items in this section have no implementation in the provided source. They are
drawn from `Improvements.md` where the underlying architecture has been
verified to not yet exist.

---

### Viewport Source Clipping / Scissoring

The renderer passes `nullptr` as the source rect to `SDL_RenderTexture`,
rendering the full canvas even when only a small region is visible.

`Viewport::getVisibleCanvasBounds()` already computes the visible world-space
region. The work needed:
1. Convert visible bounds to canvas-space.
2. Pass as source rect to `SDL_RenderTexture`.
3. Adjust the destination rect accordingly.

This is an optimisation, not a correctness fix. Profile before implementing.

---

### Window Resize Support (Viewport + Editor)

Beyond the immediate viewport bug (AUDIT-010), full resize support would
require:
1. Handling `SDL_EVENT_WINDOW_RESIZED` in `App`
2. Updating `m_viewport.setScreenRect()`
3. Optionally re-centering or re-fitting the canvas

Currently `Editor::resizeCanvas()` calls `centerCanvas()` correctly using
`SDL_GetRenderOutputSize`. The same approach would work for window resize.

---

### Canvas Resize UX

`Editor::resizeCanvas()` is functional and used (bound to Ctrl+`=` and
Ctrl+`-`). A UI panel for entering an arbitrary target size does not exist.
Adding an ImGui window in `Editor::renderUI()` that exposes width and height
inputs and calls `resizeCanvas()` would be self-contained.

---

### Tool Color Selection

All tools have hardcoded or default colors (`COLORS::BLACK` for Pencil and
Line, `COLORS::WHITE` for Eraser). No UI exists to change them. Adding a
color picker ImGui panel and wiring it to each tool's `color` member is a
bounded, self-contained task.

---

### Fill Tool

`InputCommand::FILL` is in the enum. A fill tool would require:
1. A correct `floodFill` implementation (see verified issue)
2. A new tool class (subclass `BaseTool`)
3. Registration in `Editor::setupTools()`
4. Binding `InputCommand::FILL` in `Editor::setupInputBindings()`

Depends on: fixing AUDIT-002 and AUDIT-003 first.

---

### PixelBuffer Abstraction

`Rasterizer` functions take `SDL_Surface*` directly. Introducing a
`PixelBuffer` abstraction (wrapping surface access via an interface) would:
- Decouple rasterization from SDL
- Enable unit testing without SDL
- Allow alternative backing stores in the future

This is a significant refactor touching every rasterizer function and every
tool. It should follow the verified-issue fixes and dead-code removals.

---

### Multi-Layer Canvas

`Canvas::m_renderTargets` is a declared but empty `std::vector<RenderTarget*>`.
This is the natural extension point for a layer system.

A layer system would require:
- A concrete Layer type with blend mode and opacity
- A compositor that merges layers to a final output surface
- Layer-aware commands that target a specific layer
- UI for layer management

Nothing in the current codebase implements any of this. The empty vector
may have been added as a placeholder.

---

### Headless Testing

No test fixtures or test runner configuration exists in the provided source.
Testing requires either:
- A mock `PixelBuffer` (after the abstraction above), or
- Initialising SDL in test mode and using real surfaces

The `LX_ASSERT` macro and `Logger` are both usable in tests as-is. The
primary blocker is the SDL dependency in `RenderTarget` and `Rasterizer`.

---

## Improvements.md Verification Status

The table below classifies every ticket from `Improvements.md` against the
source code.

| Ticket | Title | Status | Notes |
|---|---|---|---|
| ticket-001 | Fix SnapshotCommand dirty rect | **Verified** | `markDirty()` confirmed; see AUDIT-005 |
| ticket-002 | Remove m_boundingBox dead state | **Verified** | No read sites found; see AUDIT-007 |
| ticket-003 | Resolve preview ownership ambiguity | **Verified** | Canvas::m_preview unused; see AUDIT-006 |
| ticket-004 | Clean up preview ownership | **Verified** | Follows from ticket-003 |
| ticket-005 | Document canvas responsibility | **Not implemented** | No docs directory in source |
| ticket-006 | Verify interaction state usage | **Verified** | modifier and interactionID unused; see AUDIT-013 |
| ticket-007 | Remove dead interaction state | **Verified** | Follows from ticket-006 |
| ticket-008 | Optimize preview clearing | **Verified** | Full clear on each move confirmed; see AUDIT-017 |
| ticket-009 | Benchmark command dirty rect | **Not implemented** | No benchmark infrastructure in source |
| ticket-010 | Verify viewport transform chain | **Verified** | All three mouse events use screenToCanvas correctly |
| ticket-011 | Measure rendering pipeline efficiency | **Not implemented** | No instrumentation in source |
| ticket-012 | Validate scissoring opportunity | **Not implemented** | getVisibleCanvasBounds unused |
| ticket-013 | Document input pipeline | **Not implemented** | No docs in source |
| ticket-014 | Verify tool SDL independence | **Verified** | Tools take vec2, no SDL_Event found in tool code |
| ticket-015 | Document tool interaction model | **Not implemented** | No docs in source |
| ticket-016 | Verify command creation timing | **Verified** | All commands created in onMouseUp only |
| ticket-017 | PixelBuffer abstraction | **Not implemented** | No PixelBuffer class exists |
| ticket-018 | Layer system architecture | **Not implemented** | m_renderTargets is empty placeholder only |
| ticket-019 | Serialization format design | **Not implemented** | No file I/O in source |
| ticket-020 | Headless testing enablement | **Not implemented** | No test infrastructure in source |
