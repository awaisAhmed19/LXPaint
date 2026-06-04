# LXPaint — Rendering Pipeline


---

## Table of Contents

1. [Overview](#overview)
2. [Components and Ownership](#components-and-ownership)
3. [RenderTarget — Surface and Texture](#rendertarget--surface-and-texture)
4. [Dirty Rectangle Tracking](#dirty-rectangle-tracking)
5. [Renderer — GPU Sync and Draw](#renderer--gpu-sync-and-draw)
6. [Rasterizer — Software Pixel Operations](#rasterizer--software-pixel-operations)
7. [Frame Render Flow](#frame-render-flow)
8. [Render Order](#render-order)
9. [Known Limitations and Verified Bugs](#known-limitations-and-verified-bugs)
10. [Extension Points](#extension-points)

---

## Overview

LXPaint uses a two-tier rendering model:

**Tier 1 — Software rasterization:** All painting algorithms run on CPU
directly against `SDL_Surface` pixel buffers. No GPU shaders are used for
drawing.

**Tier 2 — GPU compositing:** The `Renderer` class uploads dirty surface
regions to GPU textures via `SDL_UpdateTexture`, then issues
`SDL_RenderTexture` calls to display them.

```
CPU (RAM)                      GPU (VRAM)
┌──────────────┐   sync()      ┌──────────────┐
│ SDL_Surface  │──────────────▶│ SDL_Texture  │
│ (canvas)     │  UpdateTexture│ (canvas)     │
└──────────────┘               └──────────────┘
                                      │ RenderTexture
┌──────────────┐   sync()      ┌──────▼───────┐
│ SDL_Surface  │──────────────▶│ SDL_Texture  │──▶  Display
│ (preview)    │               │ (preview)    │
└──────────────┘               └──────────────┘
```

---

## Components and Ownership

| Component | Owner | Responsibility |
|---|---|---|
| `RenderTarget` | base class | owns SDL_Surface + SDL_Texture; tracks dirty rect |
| `Canvas : RenderTarget` | `Editor` | painting surface |
| `PreviewLayer : RenderTarget` | `Editor` | transparent overlay for shape preview |
| `Renderer` | `Editor` (value member) | surface→texture sync; SDL_RenderTexture calls |
| `SDL_Renderer*` | `App::Window` | GPU context; Renderer holds non-owning reference |

Evidence: `Editor.h`, `Renderer.h`, `Window.h`

---

## RenderTarget — Surface and Texture

`RenderTarget` owns the CPU-side surface and the GPU-side texture as a matched
pair.

```
RenderTarget
  m_surface:       SDL_Surface*   ARGB8888, CPU pixels
  m_texture:       SDL_Texture*   SDL_TEXTUREACCESS_STATIC, initially null
  m_width:         int
  m_height:        int
  m_textureWidth:  int            last width the texture was created for
  m_textureHeight: int            last height the texture was created for
  m_dirty:         bool
  m_dirtyRect:     SDL_Rect
  m_boundingBox:   SDL_Rect       public; see Known Limitations
```

### Construction

```cpp
RenderTarget(int w, int h)
  → SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ARGB8888)
```

The texture is **not** created at construction. It is created lazily by
`Renderer::ensureTexture()` on the first render call.

### Allocation vs Resize

`allocate(int w, int h, FillColor fill)`:
- Creates a new surface filled with the specified color.
- Destroys the previous surface.
- Destroys the GPU texture (forces lazy recreation).
- Calls `markDirty()`.

`resize(int w, int h)`:
- Creates a new surface filled white.
- Blits the overlapping region from the old surface.
- Destroys the old surface and GPU texture.
- Calls `markDirty()`.

`swapTarget(RenderTarget& other)`:
- Exchanges surfaces, textures, and dimensions via `std::swap`.
- Sets `m_dirty = true` on both targets.
- Used by `Canvas::resize()` to atomically swap the new canvas in.

Evidence: `RenderTarget.cpp`

---

## Dirty Rectangle Tracking

`RenderTarget` tracks which region of the surface has changed since the last
GPU upload.

### How Dirty Rects Are Set

| Method | Effect |
|---|---|
| `markDirty()` | Sets dirty rect to full surface `{0, 0, m_width, m_height}` |
| `invalidateRect(SDL_Rect)` | If not dirty: sets dirty rect to rect. If already dirty: unions with existing dirty rect via `SDL_GetRectUnion` |
| `clear()` | Calls `SDL_FillSurfaceRect` then `markDirty()` |
| `clearRGBA()` | Calls `clear()` |
| `allocate()` | Calls `markDirty()` |
| `resize()` | Calls `markDirty()` |
| `blitFrom()` | Calls `markDirty()` |

### How Dirty Rects Are Consumed

`Renderer::sync()` calls `target.getDirtyRect()`, uses it in
`SDL_UpdateTexture`, then calls `target.clearDirty()` which resets
`m_dirty = false` and `m_dirtyRect = {0,0,0,0}`.

### Stroke Tools: invalidateRect path

Pencil and Eraser call `canvas->invalidateRect(segmentRect)` on each mouse
move, where `segmentRect` is the bounding box of the current line segment.
This accumulates via `SDL_GetRectUnion` into a growing dirty rect over the
stroke.

### Geometric Tools: markDirty path

Line, Rect, and Circle call `canvas->markDirty()` in `onMouseUp`, which sets
the dirty rect to the full canvas.

Evidence: `RenderTarget.cpp`, `Pencil.cpp`, `Eraser.cpp`, `Line.cpp`,
`Rect.cpp`, `Circle.cpp`

---

## Renderer — GPU Sync and Draw

```
Renderer
  m_renderer: SDL_Renderer*   (non-owning, provided at construction)
```

### ensureTexture(RenderTarget&)

Creates or recreates the GPU texture when:
- `m_texture` is null, or
- Stored texture dimensions do not match current surface dimensions.

```cpp
target.m_texture = SDL_CreateTexture(
    m_renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STATIC,
    target.getWidth(),
    target.getHeight()
);
SDL_SetTextureBlendMode(target.m_texture, SDL_BLENDMODE_BLEND);
```

`SDL_BLENDMODE_BLEND` is set on all textures, which means the preview layer's
alpha channel is respected during compositing.

### sync(RenderTarget&)

```
sync(target)
  ├── ensureTexture(target)
  ├── if !target.isDirty() → return (skip upload)
  ├── dirty = target.getDirtyRect()
  ├── if dirty.w <= 0 || dirty.h <= 0:
  │       dirty = {0, 0, target.getWidth(), target.getHeight()}  ← full fallback
  ├── pixels = surface->pixels + dirty.y * pitch + dirty.x * 4
  └── SDL_UpdateTexture(texture, &dirty, pixels, pitch)
      └── target.clearDirty()
```

The full-fallback case triggers when `getDirtyRect()` returns a zero-size
rect while `isDirty()` is true. This would happen if `m_dirty` is set without
a valid dirty rect, which does not occur in the current codebase.

### renderTarget(RenderTarget&, Viewport&, Transform2D&)

```
renderTarget(target, viewport, transform)
  ├── if target.isDirty() → sync(target)
  ├── if !target.getTexture() → log error, return
  ├── mainRect = {transform.position.x, transform.position.y,
  │               target.getWidth(), target.getHeight()}
  ├── dst = viewport.worldRectToScreen(mainRect)
  └── SDL_RenderTexture(m_renderer, texture, nullptr, &dst)
```

The `srcRect` argument to `SDL_RenderTexture` is `nullptr`, meaning the full
texture is always sampled. No source clipping is performed; see
[Known Limitations](#known-limitations-and-verified-bugs).

Evidence: `Renderer.cpp`

---

## Rasterizer — Software Pixel Operations

All functions are in the `Rasterizer` namespace. They take `SDL_Surface*`
directly and write pixels in ARGB8888.

### Available Functions

| Function | Algorithm |
|---|---|
| `bresenham(start, end, surface, color, brushSize, useXOR)` | Bresenham integer line; selects horizontal or vertical span based on slope |
| `dda(start, end, surface, color, brushSize, useXOR)` | DDA floating-point line |
| `drawEllipse(surface, xc, yc, rx, ry, color)` | Midpoint ellipse algorithm |
| `drawEllipse_theta(surface, x, y, w, h, color)` | Theta-step polygon approximation; used by Circle tool |
| `drawCircle(surface, x_centre, y_centre, r, color)` | Midpoint circle algorithm |
| `drawRectStroke(surface, a, b, color, brushSize)` | Four Bresenham lines |
| `drawRectFill(surface, a, b, color, isWhite)` | Stroke + flood fill |
| `drawPixel(surface, x, y, color)` | Single pixel, bounds-checked |
| `drawVerticalSpan(surface, x, y, thickness, color, useXOR)` | Vertical brush stamp |
| `drawHorizontalSpan(surface, x, y, thickness, color, useXOR)` | Horizontal brush stamp |
| `rectFill(surface, minX, minY, maxX, maxY, color)` | Filled rectangle |
| `rectFillWhite(surface, minX, minY, maxX, maxY)` | Interior-only white fill |
| `floodFillHelper(surface, pos, color, newColor)` | Recursive flood fill (internal) |

`floodFill` is **declared** in `Rasterizer.h` but its implementation is not
present in `Rasterizer.cpp`. Only `floodFillHelper` is implemented.

### Bresenham Span Selection

```cpp
if (steep) {
    // dy > dx: stepping in Y, use horizontal span for X brush coverage
    drawHorizontalSpan(surface, x1, y1, brushSize, color, useXOR);
} else {
    // dx >= dy: stepping in X, use vertical span for Y brush coverage
    drawVerticalSpan(surface, x1, y1, brushSize, color, useXOR);
}
```

`steep = abs(y2 - y1) > abs(x2 - x1)`

### Surface Locking

Both `dda()` and `bresenham()` call `lockSurface()` at entry and
`unlockSurface()` at exit. `lockSurface` is defined in `Globals.h`.

**Verified bug:** `lockSurface` uses `SDL_LockSurface(surface) < 0` which
does not detect failure in SDL3. See [Known Limitations](#known-limitations-and-verified-bugs).

### useXOR Mode

Both line functions support an XOR drawing mode for temporary overlays.
When `useXOR = true`, the pixel value is XOR'd against the color's RGB
channels (`color & 0x00FFFFFF`). No tools in the provided source pass
`useXOR = true`; all calls pass `false`.

Evidence: `Rasterizer.cpp`, `Rasterizer.h`

---

## Frame Render Flow

```
Application::render()
│
├── ImGui_ImplSDLRenderer3_NewFrame()
├── ImGui_ImplSDL3_NewFrame()
├── ImGui::NewFrame()
│
├── SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255)
├── SDL_RenderClear(renderer)   ← grey background
│
├── Editor::render()
│   ├── Renderer::renderTarget(m_canvas, m_viewport, m_docTransform)
│   │   ├── sync(m_canvas)  if dirty
│   │   │   ├── ensureTexture  if needed
│   │   │   └── SDL_UpdateTexture(dirty region)
│   │   └── SDL_RenderTexture(canvas_texture, dst_rect)
│   │
│   └── if activeTool != null && activeTool->usesPreview():
│       └── Renderer::renderTarget(m_preview, m_viewport, m_docTransform)
│           ├── sync(m_preview)  if dirty
│           └── SDL_RenderTexture(preview_texture, dst_rect)
│
├── Editor::renderUI()
│   └── ImGui::Begin("History") ... ImGui::End()
│
├── ImGui::Render()
├── ImGui_ImplSDLRenderer3_RenderDrawData(...)
└── SDL_RenderPresent(renderer)
```

---

## Render Order

| Order | What | Blend |
|---|---|---|
| 1 | Grey background (SDL_RenderClear) | opaque |
| 2 | Canvas texture | opaque (ARGB, fully opaque pixels after white init) |
| 3 | Preview texture (tools that return `usesPreview() = true`) | `SDL_BLENDMODE_BLEND` |
| 4 | ImGui panels | ImGui-managed |

**Which tools use preview:**

| Tool | `usesPreview()` | Evidence |
|---|---|---|
| Pencil | false (default) | `BaseTool.h` |
| Eraser | false (default) | `BaseTool.h` |
| Line | true | `Line.h` |
| Rect | true | `Rect.h` |
| Circle | true | `Circle.h` |

Pencil and Eraser draw directly to the canvas during drag; they do not use the
preview layer.

---

## Known Limitations and Verified Bugs

### 1. lockSurface Silently Ignores Failure on SDL3 (Verified)

`Globals.h::lockSurface()`:

```cpp
if (SDL_LockSurface(surface) < 0)   // BUG
    return false;
```

In SDL3, `SDL_LockSurface` returns `bool`: `true` on success, `false` on
failure.

`false < 0` evaluates to `0 < 0 = false`.

When locking fails, the condition is never entered, and `lockSurface` returns
`true`. All subsequent pixel writes in `bresenham()` and `dda()` proceed
against an unlocked surface.

Fix: `if (!SDL_LockSurface(surface)) return false;`

Evidence: `Globals.h`, SDL3 API documentation

---

### 2. floodFill Declared But Not Implemented (Verified)

`Rasterizer.h` declares:
```cpp
void floodFill(SDL_Surface* surface, vec2 pos, uint32_t color, uint32_t newcolor);
```

Only `floodFillHelper` exists in `Rasterizer.cpp`. The public `floodFill`
function body is absent. Any call to `floodFill` would fail at link time.

Evidence: `Rasterizer.h` (declaration), `Rasterizer.cpp` (no definition)

---

### 3. floodFillHelper Has Multiple Logic Bugs (Verified)

```cpp
void floodFillHelper(SDL_Surface *surface, vec2 pos, uint32_t color, uint32_t newcolor) {
    int m = surface->h;   // m = HEIGHT
    int n = surface->w;   // n = WIDTH
    float x = pos.x;
    float y = pos.y;
    ...
    if (x + 1 < m)        // BUG 1: compares x (column) against height
        floodFillHelper(surface, {x - 1.0f, y}, ...);  // BUG 2: moves LEFT, not right
    if (y + 1 < n)        // BUG 3: compares y (row) against width
        floodFillHelper(surface, {x, y - 1.0f}, ...);  // BUG 4: moves UP, not down
}
```

Bugs:
- `m` and `n` are assigned to wrong dimensions (height/width swapped in names)
- The bounds checks use wrong variables (`x` vs height, `y` vs width)
- The recursive directions are wrong (move left instead of right, move up instead of down)
- No stack overflow protection for large fills

This function is also unreachable through the public API (bug 1 above).

Evidence: `Rasterizer.cpp::floodFillHelper()`

---

### 4. drawRectFill Signature Mismatch (Verified)

Header `Rasterizer.h`:
```cpp
void drawRectFill(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                  int brushSize, bool isWhiteFill);
```

Implementation `Rasterizer.cpp`:
```cpp
void drawRectFill(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                  bool isWhite)
```

The header has an extra `int brushSize` parameter that the implementation does
not. The last parameter is also named differently. This is a silent ABI
mismatch. Any call to `drawRectFill` that passes a `brushSize` would
misinterpret parameters.

No call site for `drawRectFill` was found in the provided tool code; `Rect`
uses `drawRectStroke` instead.

Evidence: `Rasterizer.h` line for `drawRectFill`, `Rasterizer.cpp::drawRectFill()`

---

### 5. No Source Clipping in Renderer (Verified)

`Renderer::renderTarget()` passes `nullptr` as the source rect to
`SDL_RenderTexture`. The full texture is sampled regardless of how much of the
canvas is visible in the viewport. At high zoom levels or small viewports, GPU
bandwidth is used for offscreen regions.

Evidence: `Renderer.cpp::renderTarget()` — `SDL_RenderTexture(m_renderer, target.getTexture(), nullptr, &dst)`

---

### 6. Renderer::begin() and Renderer::end() are Inert (Verified)

Both methods are declared in `Renderer.h` but their implementations are not
present in `Renderer.cpp`. Neither is called anywhere in `App.cpp` or
`Editor.cpp`.

Evidence: `Renderer.h` (declarations), `Renderer.cpp` (absent), `App.cpp`
(not called)

---

## Extension Points

### Adding a New Rasterization Function

Add a declaration to `Rasterizer.h` and an implementation to `Rasterizer.cpp`.
The function should accept `SDL_Surface*` and call `lockSurface()` /
`unlockSurface()` around pixel writes (after fixing the SDL3 lock bug).
Bounds-check with `surface->w` and `surface->h`.

### Uploading Only a Dirty Sub-Region

`Renderer::sync()` already reads `target.getDirtyRect()`. Tools that call
`canvas->invalidateRect(rect)` instead of `canvas->markDirty()` will
automatically benefit from partial uploads. The infrastructure is in place;
the remaining work is switching geometric tool mouse-up paths and
`SnapshotCommand::undo()`/`redo()` to use `invalidateRect`.

### Adding a Render Layer

The `Editor::render()` method currently renders two surfaces in sequence
(canvas, then preview). To add a third layer, create a `RenderTarget` owned
by the `Editor` and call `m_renderer.renderTarget(newLayer, ...)` between or
after the existing calls.
