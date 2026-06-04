# LXPaint — Viewport

---

## Table of Contents

1. [Overview](#overview)
2. [Viewport State](#viewport-state)
3. [Coordinate Spaces](#coordinate-spaces)
4. [Transform Functions](#transform-functions)
5. [Pan](#pan)
6. [Zoom](#zoom)
7. [Canvas Bounds and Visibility](#canvas-bounds-and-visibility)
8. [Interaction with the Editor](#interaction-with-the-editor)
9. [Interaction with the Renderer](#interaction-with-the-renderer)
10. [Known Limitations](#known-limitations)
11. [Extension Points](#extension-points)

---

## Overview

The `Viewport` class manages the relationship between screen pixels and canvas
pixels. It tracks pan (translation) and zoom (scale), applies them to convert
mouse positions into canvas coordinates, and converts canvas-space rects into
screen-space rects for rendering.

`Viewport` is owned by `Editor` as a value member. It holds no SDL resources
and has no destructors with side effects.

---

## Viewport State

```
Viewport
  m_pan:          vec2    current pan offset in screen pixels
  m_zoom:         float   current zoom factor (clamped MIN 0.1 MAX 16.0)
  m_screenRect:   SDL_FRect   the screen region the viewport occupies
  m_canvasWidth:  int     canvas width in pixels (set via onCanvasResized)
  m_canvasHeight: int     canvas height in pixels (set via onCanvasResized)
```

Constants defined in `Viewport.cpp`:
```cpp
#define MIN_ZOOM  0.1f
#define MAX_ZOOM  16.0f
#define MARGIN    0.95f   // used only in fitCanvasToScreen()
```

---

## Coordinate Spaces

Three spaces are used in the application. Only `Viewport` performs the
transitions between them.

```
┌─────────────────────────────────────────────────┐
│  Screen Space                                   │
│  Origin: top-left of the OS window              │
│  Units:  raw pixel coordinates from SDL events  │
│  Users:  InputDispatcher (mouse pos), Renderer  │
└────────────────────┬────────────────────────────┘
                     │  screenToWorld(screen)
                     │  screen→world: (screen - pan) / zoom
                     ▼
┌─────────────────────────────────────────────────┐
│  World Space                                    │
│  Origin: top-left of the viewport (pan = 0)     │
│  Units:  canvas pixels at zoom 1.0              │
│  Users:  worldRectToScreen, worldToScreen       │
└────────────────────┬────────────────────────────┘
                     │  subtract docTransform.position
                     │  (done in screenToCanvas)
                     ▼
┌─────────────────────────────────────────────────┐
│  Canvas Space                                   │
│  Origin: top-left of the canvas surface         │
│  Units:  surface pixels (integer)               │
│  Users:  tools, rasterizer, commands            │
└─────────────────────────────────────────────────┘
```

`docTransform` is a `Transform2D` owned by `Editor`. Its `position` holds the
canvas offset in world space. In the current codebase it is always `{0.0f,
0.0f}` because `Editor` uses pan to center the canvas rather than moving the
transform. `screenToCanvas` subtracts `docTransform.position` as part of its
computation, making the system ready for non-zero document offsets.

Evidence: `Viewport.cpp`, `Editor.cpp` — `m_docTransform({0.0f, 0.0f})`

---

## Transform Functions

### screenToWorld

```cpp
vec2 screenToWorld(vec2 screen) const
  return { (screen.x - m_pan.x) / m_zoom,
           (screen.y - m_pan.y) / m_zoom }
```

Asserts `m_zoom > 0.0f`.

### screenToCanvas

```cpp
vec2 screenToCanvas(vec2 screen, const Transform2D& docTransform) const
  vec2 world = screenToWorld(screen)
  return { world.x - docTransform.position.x,
           world.y - docTransform.position.y }
```

This is the function called by `Editor::handleEvent()` for all three mouse
events (down, move, up). Every tool receives canvas-space coordinates only.

### worldToScreen

```cpp
vec2 worldToScreen(vec2 world) const
  return { world.x * m_zoom + m_pan.x,
           world.y * m_zoom + m_pan.y }
```

Inverse of `screenToWorld`.

### worldRectToScreen

```cpp
SDL_FRect worldRectToScreen(SDL_FRect rect) const
  return SDL_FRect {
    rect.x * m_zoom + m_pan.x,
    rect.y * m_zoom + m_pan.y,
    rect.w * m_zoom,
    rect.h * m_zoom
  }
```

Used by `Renderer::renderTarget()` to compute the destination rect for
`SDL_RenderTexture`.

### isPointInCanvas

```cpp
bool isPointInCanvas(vec2 screenPos, const Transform2D& docTransform) const
  vec2 canvasPos = screenToCanvas(screenPos, docTransform)
  return canvasPos.x >= 0.0f && canvasPos.x < m_canvasWidth &&
         canvasPos.y >= 0.0f && canvasPos.y < m_canvasHeight
```

Called by `Editor::handleEvent()` at mouse-down to discard clicks outside the
canvas.

Evidence: `Viewport.cpp`, `Viewport.h`

---

## Pan

### Setting Pan

```cpp
void setPan(vec2 pan)
  m_pan = pan
  clampPan()
```

### Clamping

`clampPan()` runs after every pan change. It constrains the pan so the canvas
cannot be dragged entirely off screen.

```
canvasScreenW = m_canvasWidth  * m_zoom
canvasScreenH = m_canvasHeight * m_zoom

maxPanX =  screenW               minPanX = screenW - canvasScreenW
maxPanY =  screenH               minPanY = screenH - canvasScreenH

m_pan.x = clamp(m_pan.x, minPanX, maxPanX)
m_pan.y = clamp(m_pan.y, minPanY, maxPanY)
```

This ensures at least one screen's worth of overlap between canvas and
viewport is always maintained.

`clampPan()` exits early if `m_canvasWidth <= 0 || m_canvasHeight <= 0`, which
is the initial state before `onCanvasResized()` is called.

### Interactive Panning

In `Editor::handleEvent()`, when `InputDispatcher::isPanning()` is true (Space
held + left mouse dragging):

```cpp
m_viewport.setPan(m_viewport.getPan() + m_input.getMouseDelta());
return;  // all other input processing skipped this frame
```

The `return` prevents tools from receiving events during pan.

Evidence: `Viewport.cpp::clampPan()`, `Editor.cpp::handleEvent()`

---

## Zoom

### ZoomAt

```cpp
void ZoomAt(vec2 screenPoint, float factor)
  float oldZoom = m_zoom
  m_zoom = clamp(m_zoom * factor, MIN_ZOOM, MAX_ZOOM)

  // Compute world position under cursor before zoom change
  vec2 worldBefore = { (screenPoint.x - m_pan.x) / oldZoom,
                       (screenPoint.y - m_pan.y) / oldZoom }

  // Adjust pan so the same world point maps back to the same screen point
  m_pan.x = screenPoint.x - worldBefore.x * m_zoom
  m_pan.y = screenPoint.y - worldBefore.y * m_zoom
  clampPan()
```

This preserves the world point under the mouse cursor during zoom, which is
the standard "zoom to cursor" behaviour.

### Zoom Factor from InputDispatcher

```cpp
// InputDispatcher.cpp
case SDL_EVENT_MOUSE_WHEEL:
  m_zoomTriggered = true;
  m_zoomFactor = e.wheel.y > 0 ? 1.1f : 0.9f;
```

Each scroll tick applies ±10% zoom. The factor is consumed by
`Editor::handleEvent()`:

```cpp
if (m_input.zoomTriggered()) {
    m_viewport.ZoomAt(m_input.getMouseScreenPos(), m_input.getZoomFactor());
}
```

Evidence: `Viewport.cpp::ZoomAt()`, `InputDispatcher.cpp`

---

## Canvas Bounds and Visibility

### onCanvasResized

```cpp
void onCanvasResized(int cW, int cH)
  LX_ASSERT(cW > 0 && cH > 0)
  m_canvasWidth  = cW
  m_canvasHeight = cH
  clampPan()
```

**Note:** `onCanvasResized` is defined and implemented, but `Editor::resizeCanvas()`
does not call it. After a canvas resize the viewport's stored dimensions are
stale until the next manual pan clamp. `isPointInCanvas()` would use the old
dimensions.

Evidence: `Viewport.cpp::onCanvasResized()`, `Editor.cpp::resizeCanvas()` —
`onCanvasResized` call is absent.

### fitCanvasToScreen

```cpp
void fitCanvasToScreen()
  zoomX = screenW / m_canvasWidth
  zoomY = screenH / m_canvasHeight
  fitZoom = min(zoomX, zoomY) * MARGIN   // MARGIN = 0.95
  setZoom(fitZoom)
  m_pan.x = (screenW - canvasScreenW) * 0.5f
  m_pan.y = (screenH - canvasScreenH) * 0.5f
```

Not called in the current main loop. `Editor::centerCanvas()` exists as an
alternative that also centers based on render output size.

### getVisibleCanvasBounds

```cpp
SDL_FRect getVisibleCanvasBounds() const
  topLeft    = screenToWorld({m_screenRect.x, m_screenRect.y})
  bottomRight = screenToWorld({m_screenRect.x + w, m_screenRect.y + h})
  return SDL_FRect{ topLeft.x, topLeft.y,
                    bottomRight.x - topLeft.x,
                    bottomRight.y - topLeft.y }
```

Returns the visible canvas region in world space. Not currently used by the
renderer; the renderer always uploads and draws the full texture. This function
provides the data needed for source clipping if that optimisation is
implemented.

### getCanvasBoundsScreen

Returns the canvas rectangle in screen space:
```cpp
worldRectToScreen({0, 0, m_canvasWidth, m_canvasHeight})
```

### isVisible

```cpp
bool isVisible(SDL_FRect rect) const
  SDL_FRect screen = worldRectToScreen(rect)
  return SDL_HasRectIntersectionFloat(&screen, &m_screenRect)
```

Not used by any caller in the provided source.

Evidence: `Viewport.cpp`, `Viewport.h`

---

## Interaction with the Editor

### During Construction (`Editor::Editor()`)

```cpp
m_viewport.setZoom(1.0f)
m_viewport.setPan({0.0f, 0.0f})
m_viewport.setScreenRect({0, 0, 1280, 720})
centerCanvas()
```

`centerCanvas()` computes the pan offset needed to center the canvas in the
render output:

```cpp
void Editor::centerCanvas()
  SDL_GetRenderOutputSize(m_renderer.getSDLRenderer(), &w, &h)
  float canvasW = m_canvas.getWidth() * zoom
  float canvasH = m_canvas.getHeight() * zoom
  m_viewport.setPan({ (w - canvasW) * 0.5f, (h - canvasH) * 0.5f })
```

### Mouse Event Handling (All Three Events)

```cpp
vec2 mousePos = m_viewport.screenToCanvas(
    m_input.getMouseScreenPos(), m_docTransform);
```

Followed by `clampToCanvas()` before passing to the tool.

### Zoom

```cpp
if (m_input.zoomTriggered())
    m_viewport.ZoomAt(m_input.getMouseScreenPos(), m_input.getZoomFactor())
```

### Pan

```cpp
if (m_input.isPanning())
    m_viewport.setPan(m_viewport.getPan() + m_input.getMouseDelta())
    return
```

Evidence: `Editor.cpp`

---

## Interaction with the Renderer

`Renderer::renderTarget()` receives the viewport and calls:

```cpp
SDL_FRect mainRect = {transform.position.x, transform.position.y,
                      target.getWidth(), target.getHeight()}
SDL_FRect dst = viewport.worldRectToScreen(mainRect)
SDL_RenderTexture(m_renderer, texture, nullptr, &dst)
```

The destination rect is fully determined by the viewport, so pan and zoom
automatically affect how the canvas is positioned and scaled on screen.

Evidence: `Renderer.cpp::renderTarget()`

---

## Known Limitations

### onCanvasResized Not Called After Resize (Verified)

`Editor::resizeCanvas()` calls `m_canvas.resize()`, `centerCanvas()`,
`m_interaction.reset()`, `m_tools.reset()`, and `m_commands.clear()`. It does
not call `m_viewport.onCanvasResized()`.

As a result, `m_canvasWidth` and `m_canvasHeight` inside the Viewport remain
at their initial values (0, from `Viewport` default construction). `clampPan`
exits early when dimensions are 0, so pan clamping is disabled after resize
until the application is restarted.

`isPointInCanvas()` uses these stale values, meaning after a resize the
canvas click boundary would be incorrect.

Evidence: `Editor.cpp::resizeCanvas()`, `Viewport.cpp::clampPan()`,
`Viewport.cpp::isPointInCanvas()`

### Viewport Initialised with Hardcoded Screen Size (Verified)

```cpp
m_viewport.setScreenRect({0, 0, 1280, 720});
```

This is set once at construction. If the window is resized, the screen rect
in the Viewport becomes stale and pan clamping would use the wrong bounds.

Evidence: `Editor.cpp` — constructor body, `App::Window::Settings::width = 1280,
height = 720`

### getVisibleCanvasBounds Not Used (Verified)

`getVisibleCanvasBounds()` and `isVisible()` exist but have no call sites in
the provided source. The renderer draws the full canvas regardless of how much
is visible in the viewport.

Evidence: `Viewport.h`/`Viewport.cpp` (definitions), `Renderer.cpp` (unused)

---

## Extension Points

### Updating Screen Rect on Window Resize

The `Viewport::setScreenRect(SDL_FRect)` setter is already available. On a
`SDL_EVENT_WINDOW_RESIZED` event, call `m_viewport.setScreenRect(...)` and
`centerCanvas()` or `fitCanvasToScreen()` as appropriate.

### Notifying Viewport After Canvas Resize

In `Editor::resizeCanvas()`, after `m_canvas.resize()` completes, add:
```cpp
m_viewport.onCanvasResized(m_canvas.getWidth(), m_canvas.getHeight());
```

### Source Clipping in Renderer

`getVisibleCanvasBounds()` returns the world-space region of the canvas that
is currently visible. This value could be transformed to canvas-space and
passed as the source rect in `SDL_RenderTexture`, uploading only the visible
portion and skipping offscreen pixels.
