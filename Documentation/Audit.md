# LXPaint — Audit

---

## Table of Contents

1. [Severity Index](#severity-index)
2. [AUDIT-001 — lockSurface Silently Ignores Failure on SDL3](#audit-001--locksurface-silently-ignores-failure-on-sdl3)
3. [AUDIT-002 — floodFill Not Implemented](#audit-002--floodfill-not-implemented)
4. [AUDIT-003 — floodFillHelper Has Multiple Logic Bugs](#audit-003--floodfillhelper-has-multiple-logic-bugs)
5. [AUDIT-004 — drawRectFill Signature Mismatch](#audit-004--drawrectfill-signature-mismatch)
6. [AUDIT-005 — SnapshotCommand Invalidates Full Canvas on Undo/Redo](#audit-005--snapshotcommand-invalidates-full-canvas-on-undoredo)
7. [AUDIT-006 — Canvas Owns an Unused PreviewLayer](#audit-006--canvas-owns-an-unused-previewlayer)
8. [AUDIT-007 — RenderTarget::m_boundingBox is Write-Only](#audit-007--rendertargetm_boundingbox-is-write-only)
9. [AUDIT-008 — Canvas::m_renderTargets is Never Populated](#audit-008--canvasm_rendertargets-is-never-populated)
10. [AUDIT-009 — Viewport Not Notified After Canvas Resize](#audit-009--viewport-not-notified-after-canvas-resize)
11. [AUDIT-010 — Viewport Screen Rect is Hardcoded](#audit-010--viewport-screen-rect-is-hardcoded)
12. [AUDIT-011 — deltaTime Computed but Not Used](#audit-011--deltatime-computed-but-not-used)
13. [AUDIT-012 — Dead Mouse Handler Declarations](#audit-012--dead-mouse-handler-declarations)
14. [AUDIT-013 — ToolInteractionState Has Unused Fields](#audit-013--toolinteractionstate-has-unused-fields)
15. [AUDIT-014 — Circle Tool Computes dx/dy and Discards Them](#audit-014--circle-tool-computes-dxdy-and-discards-them)
16. [AUDIT-015 — Stroke Tools Duplicate Full Canvas at Mouse-Down](#audit-015--stroke-tools-duplicate-full-canvas-at-mouse-down)
17. [AUDIT-016 — Renderer::begin() and end() Are Inert](#audit-016--rendererbegin-and-end-are-inert)
18. [AUDIT-017 — Preview Cleared Full Surface on Every Mouse Move](#audit-017--preview-cleared-full-surface-on-every-mouse-move)

---

## Severity Index

| ID | Title | Class | Severity |
|---|---|---|---|
| 001 | lockSurface ignores failure on SDL3 | Bug | High — silent corruption |
| 002 | floodFill not implemented | Bug | High — linker failure if called |
| 003 | floodFillHelper logic bugs | Bug | High — wrong behavior |
| 004 | drawRectFill signature mismatch | Mismatch | Medium — wrong parameters if called |
| 005 | SnapshotCommand full-canvas dirty | Design | Medium — unnecessary GPU upload |
| 006 | Canvas unused PreviewLayer | Dead Code | Low — memory waste |
| 007 | m_boundingBox write-only | Dead Code | Low — dead public state |
| 008 | m_renderTargets never populated | Dead Code | Low — dead declaration |
| 009 | Viewport not notified after resize | Bug | Medium — stale state |
| 010 | Viewport screen rect hardcoded | Design | Medium — breaks on window resize |
| 011 | deltaTime computed but unused | Dead Code | Low — dead variable |
| 012 | Dead mouse handler declarations | Dead Code | Low — misleading header |
| 013 | ToolInteractionState unused fields | Dead Code | Low — dead state |
| 014 | Circle dx/dy unused variables | Dead Code | Low — dead variables |
| 015 | Stroke tools full-canvas duplicate | Design | Low — memory cost per stroke |
| 016 | Renderer begin/end are inert | Dead Code | Low — dead API |
| 017 | Preview full clear every move | Design | Low — unnecessary work |

---

## AUDIT-001 — lockSurface Silently Ignores Failure on SDL3

**Classification:** Bug  
**Severity:** High

### Evidence

`src/App/Globals.h`:
```cpp
inline bool lockSurface(SDL_Surface *surface) {
    if (!surface) return false;
    if (SDL_MUSTLOCK(surface)) {
        if (SDL_LockSurface(surface) < 0)   // ← BUG
            return false;
    }
    return true;
}
```

In SDL3, `SDL_LockSurface` returns `bool`: `true` = success, `false` = failure.

`false < 0` evaluates to `0 < 0 = false`.

When the lock fails:
1. The `< 0` condition is never true.
2. `false` is never returned.
3. `lockSurface` returns `true`.
4. `Rasterizer::bresenham()` and `Rasterizer::dda()` proceed to write pixels
   against an unlocked surface.

### Counter-Evidence

If `SDL_MUSTLOCK` returns false for the surfaces used (software surfaces may
not require locking), this code path is never reached and the bug is dormant.
In practice, ARGB8888 software surfaces created with `SDL_CreateSurface` may
not require locking, making this a latent rather than active bug. However, the
condition is still incorrectly written.

### Recommendation

```cpp
if (!SDL_LockSurface(surface))
    return false;
```

---

## AUDIT-002 — floodFill Not Implemented

**Classification:** Bug  
**Severity:** High (link-time failure if called)

### Evidence

`src/Rendering/Rasterizer.h`:
```cpp
void floodFill(SDL_Surface *surface, vec2 pos, uint32_t color, uint32_t newcolor);
```

`src/Rendering/Rasterizer.cpp`: No definition of `floodFill`. Only
`floodFillHelper` exists.

No call site for `floodFill` exists in the provided source. It is declared but
would cause a linker error if called.

### Counter-Evidence

The fill tool may not be in active use. `InputCommand::FILL` exists in
`InputDispatcher.h` but has no binding in `Editor::setupInputBindings()`.

### Recommendation

Either implement `floodFill` as a thin wrapper over a corrected
`floodFillHelper`, or remove the declaration until the feature is needed.

---

## AUDIT-003 — floodFillHelper Has Multiple Logic Bugs

**Classification:** Bug  
**Severity:** High

### Evidence

`src/Rendering/Rasterizer.cpp`:
```cpp
void floodFillHelper(SDL_Surface *surface, vec2 pos, uint32_t color,
                     uint32_t newcolor) {
    uint32_t *pixels = getPixels(surface);
    int pitch = getPitch(surface);
    int m = surface->h;   // m = HEIGHT (named as if it were rows)
    int n = surface->w;   // n = WIDTH  (named as if it were columns)
    float x = pos.x;
    float y = pos.y;
    uint32_t currColor = pixels[(int)y * pitch + (int)x];

    if (currColor == color) {
        pixels[(int)y * pitch + (int)x] = newcolor;

        if (x >= 1)
            floodFillHelper(surface, {x - 1.0f, y}, color, newcolor); // move left ✓

        if (y >= 1)
            floodFillHelper(surface, {x, y - 1.0f}, color, newcolor); // move up ✓

        if (x + 1 < m)   // BUG 1: x is a column; should compare to n (width)
            floodFillHelper(surface, {x - 1.0f, y}, color, newcolor); // BUG 2: moves LEFT again

        if (y + 1 < n)   // BUG 3: y is a row; should compare to m (height)
            floodFillHelper(surface, {x, y - 1.0f}, color, newcolor); // BUG 4: moves UP again
    }
}
```

Bugs:
1. `x + 1 < m` should be `x + 1 < n` (width, not height)
2. The recursive call moves to `{x - 1, y}` (left) instead of `{x + 1, y}` (right)
3. `y + 1 < n` should be `y + 1 < m` (height, not width)
4. The recursive call moves to `{x, y - 1}` (up) instead of `{x, y + 1}` (down)
5. No protection against stack overflow on large fills

### Counter-Evidence

This function is unreachable through the public API (AUDIT-002), so these bugs
have no runtime impact today.

### Recommendation

Rewrite the function entirely. A stack-based (iterative) flood fill is strongly
preferred over recursive to avoid stack overflow.

---

## AUDIT-004 — drawRectFill Signature Mismatch

**Classification:** Mismatch  
**Severity:** Medium

### Evidence

`src/Rendering/Rasterizer.h` declaration:
```cpp
void drawRectFill(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                  int brushSize, bool isWhiteFill);
```

`src/Rendering/Rasterizer.cpp` definition:
```cpp
void drawRectFill(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                  bool isWhite)
```

The header has a `brushSize` parameter between `color` and `isWhiteFill` that
the implementation does not. The last parameter name also differs.

A call matching the header would pass an integer where the implementation
expects a bool, silently misinterpreting `isWhiteFill`.

### Counter-Evidence

No call site for `drawRectFill` exists in the provided tool code. `Rect.cpp`
uses `drawRectStroke`, not `drawRectFill`. The mismatch is currently dormant.

### Recommendation

Reconcile the header and implementation. Either add `brushSize` to the
implementation or remove it from the header.

---

## AUDIT-005 — SnapshotCommand Invalidates Full Canvas on Undo/Redo

**Classification:** Design Issue  
**Severity:** Medium

### Evidence

`src/Editor/Commands/SnapshotCommand.h`:
```cpp
void undo(Canvas &canvas) override {
    LX_ASSERT(m_before != nullptr, "Undo snapshot missing");
    restoreRegion(canvas.getSurface(), m_before.get(), m_bounds);
    canvas.markDirty();   // ← full canvas invalidated
}

void redo(Canvas &canvas) override {
    LX_ASSERT(m_after != nullptr, "Redo snapshot missing");
    restoreRegion(canvas.getSurface(), m_after.get(), m_bounds);
    canvas.markDirty();   // ← full canvas invalidated
}
```

`RenderTarget::markDirty()`:
```cpp
void RenderTarget::markDirty() {
    m_dirty = true;
    m_dirtyRect = {0, 0, m_width, m_height};   // full surface
}
```

`m_bounds` is available in both functions and contains the exact affected
region.

`RenderTarget::invalidateRect(m_bounds)` would upload only the changed region
to the GPU.

### Counter-Evidence

On an 800×550 canvas the full upload is ~1.76 MB. For typical interactive use
this may not be perceptible. The impact grows with canvas size.

### Recommendation

Replace `canvas.markDirty()` with `canvas.invalidateRect(m_bounds)` in both
`undo()` and `redo()`.

---

## AUDIT-006 — Canvas Owns an Unused PreviewLayer

**Classification:** Dead Code  
**Severity:** Low

### Evidence

`src/Document/Canvas.h`:
```cpp
class Canvas : public RenderTarget {
    std::vector<RenderTarget *> m_renderTargets;
    PreviewLayer m_preview;   // ← instance 1
};
```

`src/Editor/Editor.h`:
```cpp
Canvas m_canvas;
PreviewLayer m_preview;       // ← instance 2 (used)
```

`src/Editor/Editor.cpp::makeToolContext()`:
```cpp
return ToolContext{.canvas = &m_canvas, .preview = &m_preview, ...};
//                                                 ^^^^^^^^^^^
//                                              Editor::m_preview
```

`src/Editor/Editor.cpp::render()`:
```cpp
m_renderer.renderTarget(m_preview, m_viewport, m_docTransform);
//                       ^^^^^^^^^^^
//                   Editor::m_preview
```

`src/Document/Canvas.cpp`:
```cpp
Canvas::Canvas(int w, int h) : RenderTarget(w, h), m_preview(w, h) { ... }

void Canvas::resize(int w, int h, const ResizePolicy &policy) {
    ...
    m_preview.allocate(w, h);   // only usage: resizes it, nothing reads it
}
```

`Canvas::m_preview` is allocated, resized, but never read by any tool or
rendered by any renderer path.

### Counter-Evidence

Resizing `Canvas::m_preview` in `Canvas::resize()` suggests it was intended
to be used. It may be a vestige of an earlier design where Canvas was
responsible for compositing.

### Recommendation

Remove `Canvas::m_preview` and the `m_preview.allocate(w, h)` call in
`Canvas::resize()`. The preview lifecycle (allocate on resize, clear on tool
end) should be managed by `Editor`, which already owns the authoritative
instance.

---

## AUDIT-007 — RenderTarget::m_boundingBox is Write-Only

**Classification:** Dead Code  
**Severity:** Low

### Evidence

`src/Document/RenderTarget.h`:
```cpp
SDL_Rect m_boundingBox = {0, 0, 0, 0};   // public member
```

Write sites (no corresponding read sites found):

| File | Function | Assignment |
|---|---|---|
| `Line.cpp:29` | `onMouseDown` | `ctx.canvas->m_boundingBox = computeLineBounds(...)` |
| `Line.cpp:37-38` | `onMouseMove` | `ctx.canvas->m_boundingBox = computeLineBounds(...)` |
| `Line.cpp:53-55` | `onMouseUp` | `ctx.canvas->m_boundingBox = computeLineBounds(...)` |
| `Rect.cpp:28-30` | `onMouseDown` | `ctx.canvas->m_boundingBox = computeRectBounds(...)` |
| `Rect.cpp:39-41` | `onMouseMove` | `ctx.canvas->m_boundingBox = computeRectBounds(...)` |
| `Rect.cpp:62-64` | `onMouseUp` | `ctx.canvas->m_boundingBox = computeRectBounds(...)` |
| `Circle.cpp:45-47` | `onMouseMove` | `ctx.canvas->m_boundingBox = computeEllipseBounds(...)` |
| `Circle.cpp:71-73` | `onMouseUp` | `ctx.canvas->m_boundingBox = computeEllipseBounds(...)` |

`RenderTarget::updateBounds()` is defined but no tool calls it; tools write
`m_boundingBox` directly. No call site for `updateBounds()` was found.

### Counter-Evidence

`m_boundingBox` is public. An external system not visible in the provided
source could read it. Given the scope of the codebase, this is unlikely but
cannot be fully ruled out from the files provided.

### Recommendation

Remove `m_boundingBox` and `updateBounds()` from `RenderTarget`. Each tool
already computes its own local bounds via `computeLineBounds`,
`computeRectBounds`, `computeEllipseBounds`. Move those to local variables or
tool state.

---

## AUDIT-008 — Canvas::m_renderTargets is Never Populated

**Classification:** Dead Code  
**Severity:** Low

### Evidence

`src/Document/Canvas.h`:
```cpp
std::vector<RenderTarget *> m_renderTargets;
```

`src/Document/Canvas.cpp`: vector is not mentioned after construction (default
initialization produces empty vector). No `push_back`, `emplace_back`, or
iteration was found in any provided source file.

### Counter-Evidence

May be a placeholder for a future layer system.

### Recommendation

Remove the member now. If a layer system is added in the future, it can be
reintroduced with a concrete design. An empty vector in a shipping class is
misleading.

---

## AUDIT-009 — Viewport Not Notified After Canvas Resize

**Classification:** Bug  
**Severity:** Medium

### Evidence

`src/Editor/Editor.cpp::resizeCanvas()`:
```cpp
void Editor::resizeCanvas(int w, int h, const ResizePolicy &policy) {
    ...
    m_canvas.resize(w, h, policy);
    centerCanvas();
    m_interaction.reset();
    m_tools.reset();
    m_commands.clear();
    // ← m_viewport.onCanvasResized(w, h) is ABSENT
}
```

`Viewport::isPointInCanvas()` uses `m_canvasWidth` and `m_canvasHeight`:
```cpp
bool isPointInCanvas(vec2 screenPos, const Transform2D &docTransform) const {
    vec2 canvasPos = screenToCanvas(screenPos, docTransform);
    return canvasPos.x >= 0.0f && canvasPos.x < m_canvasWidth &&
           canvasPos.y >= 0.0f && canvasPos.y < m_canvasHeight;
}
```

After resize, `m_canvasWidth` and `m_canvasHeight` remain at their
construction-time values (0, since the default `Viewport` constructor leaves
them at 0).

`clampPan()` also exits early when `m_canvasWidth <= 0`.

In practice, the viewport's canvas dimensions are set by `onCanvasResized` in
`Viewport.cpp` but that function is never called by the Editor in the provided
source.

### Counter-Evidence

`centerCanvas()` calls `SDL_GetRenderOutputSize` and recomputes pan using
`m_canvas.getWidth()` directly, so visual centering still works after resize.
The failure only surfaces if the user clicks at the edge of the new canvas.

### Recommendation

Add `m_viewport.onCanvasResized(m_canvas.getWidth(), m_canvas.getHeight())`
after the `m_canvas.resize()` call in `Editor::resizeCanvas()`.

---

## AUDIT-010 — Viewport Screen Rect is Hardcoded

**Classification:** Design Issue  
**Severity:** Medium

### Evidence

`src/Editor/Editor.cpp` — constructor:
```cpp
m_viewport.setScreenRect({0, 0, 1280, 720});
```

`App::Window::Settings::width` and `Settings::height` default to 1280 and
720. The window is created as resizable:
```cpp
SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE
```

No `SDL_EVENT_WINDOW_RESIZED` handler exists in the provided source. After
a window resize, `clampPan()` would use the wrong `screenW` / `screenH`,
and `isVisible()` would use the wrong bounds.

### Counter-Evidence

If the window is not resized, this has no effect. For a paint application
that primarily targets desktop fixed-window use, this may be acceptable for
now.

### Recommendation

Handle `SDL_EVENT_WINDOW_RESIZED` in `App::Application::handleEvents()`,
retrieve the new size with `SDL_GetRenderOutputSize`, and call
`m_viewport.setScreenRect(...)`.

---

## AUDIT-011 — deltaTime Computed but Not Used

**Classification:** Dead Code  
**Severity:** Low

### Evidence

`src/App/App.cpp::run()`:
```cpp
uint64_t last = SDL_GetTicks();
while (m_running) {
    uint64_t now = SDL_GetTicks();
    float deltaTime = (now - last) / 1000.0f;   // computed
    last = now;
    handleEvents();
    m_editor->update();   // deltaTime not passed
    render();
}
```

`Editor::update()` signature: `void update()` — takes no parameters.

### Counter-Evidence

Computing delta time costs nothing and is the correct first step for
game-loop timing. It may be retained as scaffolding for future animation.

### Recommendation

Either pass `deltaTime` to `Editor::update(float dt)` and use it, or remove
the variable to avoid confusion.

---

## AUDIT-012 — Dead Mouse Handler Declarations

**Classification:** Dead Code  
**Severity:** Low

### Evidence

`src/Editor/Editor.h`:
```cpp
void handleMouseDown(const SDL_Event &e);
void handleMouseMove(const SDL_Event &e);
void handleMouseUp(const SDL_Event &e);
```

None of these functions have a definition in `Editor.cpp`. Mouse handling is
performed inline inside `Editor::handleEvent()`.

### Counter-Evidence

These may be declared with intent to refactor `handleEvent()` in the future.

### Recommendation

Remove the declarations until the refactor is planned. Undeclared private
functions do not inhibit adding them later.

---

## AUDIT-013 — ToolInteractionState Has Unused Fields

**Classification:** Dead Code  
**Severity:** Low

### Evidence

`src/Editor/Interaction/ToolInteractionState.h`:
```cpp
struct ToolInteractionState {
    bool active = false;
    vec2 startMousePos{0.0f, 0.0f};
    vec2 currMousePos{0.0f, 0.0f};
    vec2 prevMousePos{0.0f, 0.0f};
    bool mouseDown = false;
    SDL_Keymod modifier = SDL_KMOD_NONE;    // ← no write or read sites found
    uint64_t interactionID = 0;             // ← no write or read sites found

    void reset() {
        ...
        modifier = SDL_KMOD_NONE;           // only write site
        interactionID = 0;                  // only write site
    }
};
```

No code reads `modifier` or `interactionID`. No code writes to them outside
of `reset()`.

### Counter-Evidence

These fields may be reserved for modifier-aware tools (e.g., Shift for
constrained lines) or interaction identification in a future multi-touch
scenario.

### Recommendation

Remove `modifier` and `interactionID` from the struct and from `reset()`. They
can be reintroduced when actually needed.

---

## AUDIT-014 — Circle Tool Computes dx/dy and Discards Them

**Classification:** Dead Code  
**Severity:** Low

### Evidence

`src/Editor/Tools/Circle.cpp::onMouseMove()`:
```cpp
int dx = (int)(pos.x - m_start.x);    // assigned, never used
int dy = (int)(pos.y - m_start.y);    // assigned, never used

int rx = (int)(pos.x - m_start.x);
int ry = (int)(pos.y - m_start.y);
```

Same pattern in `onMouseUp()`.

`dx` and `dy` are identical to `rx` and `ry` but are never referenced. A
compiler with `-Wunused-variable` would warn on these.

### Recommendation

Remove the `dx` and `dy` declarations.

---

## AUDIT-015 — Stroke Tools Duplicate the Full Canvas at Mouse-Down

**Classification:** Design Issue  
**Severity:** Low

### Evidence

`src/Editor/Tools/Pencil.cpp::onMouseDown()`:
```cpp
m_backupSurface = SDL_DuplicateSurface(ctx.canvas->getSurface());
```

`src/Editor/Tools/Eraser.cpp::onMouseDown()`:
```cpp
m_backupSurface = SDL_DuplicateSurface(ctx.canvas->getSurface());
```

`SDL_DuplicateSurface` allocates a full surface copy. On an 800×550 ARGB8888
canvas, this is `800 * 550 * 4 = 1,760,000 bytes ≈ 1.7 MB` per stroke start.

This backup is used only to extract the `before` region at mouse-up via
`SnapshotCommand::copyRegion(m_backupSurface, m_strokeBounds)`.

Alternatively, the before-region could be captured at mouse-down once the
initial bounding box is known, avoiding the full duplicate.

### Counter-Evidence

The full duplicate is simple and correct. At mouse-down the final stroke
bounds are unknown, making a partial capture at that point require a
subsequent expansion. The current approach sacrifices memory for simplicity.
For typical canvas sizes this cost is acceptable.

### Recommendation

Defer any change until profiling shows this to be a real bottleneck.

---

## AUDIT-016 — Renderer::begin() and end() Are Inert

**Classification:** Dead Code  
**Severity:** Low

### Evidence

`src/Rendering/Renderer.h`:
```cpp
void begin();
void end();
```

Neither function is defined in `Renderer.cpp`. Neither is called anywhere in
`App.cpp` or `Editor.cpp`.

### Recommendation

Remove the declarations from `Renderer.h` until they have a purpose.

---

## AUDIT-017 — Preview Cleared Full Surface on Every Mouse Move

**Classification:** Design Issue  
**Severity:** Low

### Evidence

`src/Editor/Tools/Line.cpp::onMouseMove()`:
```cpp
ctx.preview->clearRGBA(0, 0, 0, 0);
```

`src/Editor/Tools/Rect.cpp::onMouseMove()`:
```cpp
ctx.preview->clearRGBA(0, 0, 0, 0);
```

`src/Editor/Tools/Circle.cpp::onMouseMove()`:
```cpp
ctx.preview->clearRGBA(0, 0, 0, 0);
```

`clearRGBA` calls `SDL_FillSurfaceRect(m_surface, nullptr, ...)` with a null
rect, filling the entire surface.

Each mouse move during a shape drag clears the entire 800×550 preview surface
(`SDL_FillSurfaceRect` on ~1.76M pixels) then redraws only the new shape.

### Counter-Evidence

On modern CPUs, clearing 1.76 MB of memory is fast (a few hundred
microseconds). At 60 fps this adds minimal overhead for typical canvas sizes.
The simplicity benefit is real.

### Recommendation

No immediate action needed. If profiling shows this as a bottleneck on large
canvases, the optimisation is to track the union of previous and current shape
bounds and clear only that region.
