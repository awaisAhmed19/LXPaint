# LXPaint — Architecture

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Directory Layout](#directory-layout)
3. [Ownership Map](#ownership-map)
4. [Subsystem Responsibilities](#subsystem-responsibilities)
5. [Object Lifetimes](#object-lifetimes)
6. [Main Loop](#main-loop)
7. [Coordinate Spaces](#coordinate-spaces)
8. [Known Structural Issues](#known-structural-issues)
9. [Extension Points](#extension-points)

---

## System Overview

LXPaint is a raster painting application built on SDL3 and Dear ImGui. The
architecture is layered: the application shell owns a window and an editor; the
editor owns all painting subsystems; tools operate through a shared context and
return commands that the editor pushes onto an undo stack.

```
┌────────────────────────────────────────────────────────┐
│                   App::Application                      │
│  Owns: Window, Editor                                   │
│  Runs: handleEvents → update → render (main loop)       │
└──────────────────────────┬─────────────────────────────┘
                           │ owns
          ┌────────────────▼───────────────────┐
          │              Editor                 │
          │  Owns: Canvas  PreviewLayer         │
          │         Renderer  CommandManager     │
          │         ToolManager  InputDispatcher │
          │         Viewport  ToolInteraction-  │
          │                   State             │
          └────────────────────────────────────┘
                    │           │           │
           ┌────────▼──┐  ┌────▼──┐  ┌────▼──────────┐
           │  Canvas   │  │ Tools │  │ CommandManager │
           │ (pixels)  │  │       │  │ (undo/redo)   │
           └───────────┘  └───────┘  └───────────────┘
```

Evidence:
- `App.h` lines 24–25: `std::unique_ptr<Editor>`, `std::unique_ptr<Window>`
- `Editor.h` lines 20–32: all Editor members declared
- `App.cpp` `Application::run()`: loop body confirmed

---

## Directory Layout

```
src/
├── App/              Bootstrap, window, global config, math types
│   ├── App.cpp/h     Application shell and main loop
│   ├── Globals.h     vec2, vec3, COLORS, Config, SDL surface utilities
│   ├── Utils.h       MATH::MouseOverPoint helper
│   ├── Window.cpp/h  SDL_Window + SDL_Renderer creation and ownership
│   └── main.cpp      CLI flag parsing, Application construction
│
├── Document/         Canvas and rendering surface types
│   ├── RenderTarget  Base class: surface, texture, dirty tracking
│   ├── Canvas        Concrete painting surface; extends RenderTarget
│   └── PreviewLayer  Transparent overlay; extends RenderTarget
│
├── Editor/           Painting orchestrator and all sub-editors
│   ├── Editor.cpp/h  Central coordinator; owns everything
│   ├── Commands/     Command pattern; undo/redo history
│   ├── Interaction/  ToolContext, ToolInteractionState
│   ├── Tools/        Tool hierarchy and ToolManager
│   └── Viewport/     Pan/zoom, coordinate transforms
│
├── Input/            SDL event → engine state translation
│   └── InputDispatcher
│
├── Rendering/        GPU sync and rasterization
│   ├── Renderer      Surface-to-texture sync, SDL_RenderTexture calls
│   ├── Rasterizer    Software pixel algorithms (Bresenham, DDA, ellipse…)
│   └── Transform2D   Position + scale + rotation struct
│
├── Systems/          Cross-cutting concerns
│   ├── Logger        Thread-safe file + console logger
│   ├── Assert        LX_ASSERT macro with breakpoint + abort
│   └── Profiler      Algorithm timing and comparison storage
│
└── UI/
    └── Console.h     ImGui debug panels (logs, FPS, profiler, benchmark)
```

---

## Ownership Map

The table below lists every major object, who constructs it, who holds the
unique_ptr or value, and when it is destroyed.

| Object | Owner | Storage | Destroyed when |
|---|---|---|---|
| `SDL_Window*` | `App::Window` | raw ptr | `Window::~Window()` |
| `SDL_Renderer*` | `App::Window` | raw ptr | `Window::~Window()` |
| `App::Window` | `App::Application` | `unique_ptr` | `Application` destroyed |
| `Editor` | `App::Application` | `unique_ptr` | `Application` destroyed |
| `Canvas` | `Editor` | value member | `Editor` destroyed |
| `PreviewLayer` (used) | `Editor` | value member | `Editor` destroyed |
| `PreviewLayer` (unused) | `Canvas` | value member | `Canvas` destroyed |
| `Renderer` | `Editor` | value member | `Editor` destroyed |
| `SDL_Renderer*` in Renderer | `App::Window` (source) | raw ptr (non-owning) | Renderer holds no ownership |
| `CommandManager` | `Editor` | value member | `Editor` destroyed |
| `ToolManager` | `Editor` | value member | `Editor` destroyed |
| Each `BaseTool` | `ToolManager` | `unique_ptr` in `map` | `ToolManager` destroyed |
| `Viewport` | `Editor` | value member | `Editor` destroyed |
| `InputDispatcher` | `Editor` | value member | `Editor` destroyed |
| `ToolInteractionState` | `Editor` | value member | `Editor` destroyed |
| `Transform2D m_docTransform` | `Editor` | value member | `Editor` destroyed |
| `SnapshotCommand` | `CommandManager` | `unique_ptr` in deque | Popped from stack or `clear()` |
| `SDL_Surface*` (canvas) | `RenderTarget` | raw ptr | `RenderTarget::~RenderTarget()` |
| `SDL_Texture*` | `RenderTarget` | raw ptr | `RenderTarget::~RenderTarget()` |
| `SDL_Surface*` (backup) | `StrokeTool` | raw ptr | `onMouseUp` or `deactivate()` |

Evidence sources:
- `App.h`: `unique_ptr` members
- `Editor.h`: all value members
- `RenderTarget.cpp`: destructor frees surface and texture
- `StrokeTool.h`: `freeBackupSurface()` called in destructor and `deactivate()`

---

## Subsystem Responsibilities

### App::Application

**Responsible for:**
- SDL initialization and teardown (`SDL_INIT_VIDEO`)
- ImGui initialization and teardown (SDL3 + SDLRenderer3 backends)
- Window and Editor construction
- Running the main loop: event polling → editor update → render

**Not responsible for:**
- Any painting logic
- Canvas management
- Input routing beyond polling SDL events

Evidence: `App.cpp` — `Application::Application()`, `Application::run()`

---

### App::Window

**Responsible for:**
- Creating `SDL_Window` and `SDL_Renderer`
- Providing raw pointers to both via `getNativeWindow()` / `getNativeRenderer()`

**Not responsible for:**
- Rendering anything
- Owning ImGui state

Evidence: `Window.cpp`

---

### RenderTarget

**Responsible for:**
- Owning a single `SDL_Surface*` and a corresponding `SDL_Texture*`
- Tracking a dirty rectangle (`m_dirtyRect`, `m_dirty`)
- Providing `allocate()`, `resize()`, `clear()`, `clearRGBA()`, `blitFrom()`, `swapTarget()`
- Marking itself dirty when pixels change

**Not responsible for:**
- Uploading texture to GPU (that is `Renderer::sync()`)
- Rendering to screen
- Knowing about the viewport

**Notes:**
- `m_texture` starts null; `Renderer::ensureTexture()` creates it lazily
- `friend class Renderer` grants access to texture members

Evidence: `RenderTarget.h`, `RenderTarget.cpp`

---

### Canvas

**Responsible for:**
- Owning the authoritative painting surface (white on construction)
- Implementing `resize()` with anchor/fill policy
- Computing source/destination rects for pixel-preserving resize

**Not responsible for:**
- Preview rendering
- Undo history
- Viewport transforms

**Structural note — two PreviewLayers:**
`Canvas` owns `PreviewLayer m_preview` (Canvas.h line 25) but this instance is
**never passed to ToolContext and never rendered**. The preview layer that tools
write to and that gets rendered is `Editor::m_preview`. See
[Known Structural Issues](#known-structural-issues).

`Canvas::m_renderTargets` (a `std::vector<RenderTarget*>`) is declared but
never populated in any provided source file. It has no read or write sites
beyond its implicit initialization.

Evidence: `Canvas.h`, `Canvas.cpp`, `Editor.cpp::makeToolContext()`,
`Editor.cpp::render()`

---

### Editor

**Responsible for:**
- Owning and coordinating all subsystems
- Translating input events into tool calls
- Converting screen coordinates to canvas coordinates before passing to tools
- Pushing returned commands onto the `CommandManager`
- Resizing the canvas on request
- Centering the canvas on startup and after resize

**Not responsible for:**
- Low-level SDL event creation
- Rasterization
- GPU uploads

**Declared but unimplemented:** `handleMouseDown()`, `handleMouseMove()`,
`handleMouseUp()` are declared in `Editor.h` but have no definition in
`Editor.cpp`. Mouse handling is performed inline inside `handleEvent()`.

Evidence: `Editor.h`, `Editor.cpp`

---

### ToolManager

**Responsible for:**
- Registering tools by string key into `std::map<std::string, unique_ptr<BaseTool>>`
- Tracking the active tool via a raw observer pointer
- Calling `deactivate()` on the outgoing tool when switching

**Not responsible for:**
- Tool input routing (Editor does this)
- Command storage

Evidence: `ToolManager.h`

---

### CommandManager

**Responsible for:**
- Maintaining undo and redo stacks (`std::deque<CommandEntry>`)
- Enforcing max undo depth (default 50) and max memory (default 256 MB)
- Trimming the front of the undo stack when limits are exceeded

**Not responsible for:**
- Creating commands
- Executing the initial operation
- Rendering

Evidence: `CommandManager.h`

---

### InputDispatcher

**Responsible for:**
- Translating raw `SDL_Event` into typed state flags
- Resetting per-frame state in `beginFrame()`
- Firing bound action callbacks for keyboard shortcuts

**Not responsible for:**
- Coordinate transforms
- Understanding what tools exist

Evidence: `InputDispatcher.h`, `InputDispatcher.cpp`

---

### Renderer

**Responsible for:**
- Lazily creating and recreating SDL textures (`ensureTexture()`)
- Uploading dirty surface regions to GPU (`SDL_UpdateTexture`)
- Issuing `SDL_RenderTexture` calls

**Not responsible for:**
- Owning `SDL_Renderer*` (it holds a non-owning pointer)
- Clearing the screen (App does that)
- ImGui rendering

`Renderer::begin()` and `Renderer::end()` are declared in `Renderer.h` but
not called anywhere in the provided source, and their implementations are not
visible. They are currently inert.

Evidence: `Renderer.h`, `Renderer.cpp`

---

### Rasterizer (namespace)

**Responsible for:**
- All software pixel operations: Bresenham line, DDA line, ellipse (midpoint
  and theta-step), circle (midpoint), flood fill, rect stroke and fill

**Not responsible for:**
- Texture uploads
- SDL_Renderer

**Known issues:** See [AUDIT.md](AUDIT.md) for verified bugs in `lockSurface`,
`floodFillHelper`, and the `drawRectFill` signature mismatch.

Evidence: `Rasterizer.cpp`, `Rasterizer.h`

---

## Object Lifetimes

```
Program start
│
├─ SDL_Init(VIDEO)
├─ Logger::init()
├─ Window created  ──────────────── SDL_Window, SDL_Renderer alive
├─ Editor created
│   ├─ Canvas(800,550)  ─────────── canvas surface allocated
│   ├─ PreviewLayer(800,550)  ───── editor preview surface allocated
│   ├─ Renderer(sdl_renderer)
│   ├─ CommandManager(50, 256)
│   ├─ ToolManager + tools registered
│   ├─ Viewport configured
│   └─ centerCanvas()
├─ ImGui initialized
│
│  ┌────────────────────────────────────────────────────┐
│  │  Main Loop (m_running == true)                     │
│  │                                                    │
│  │  handleEvents()                                    │
│  │    SDL_PollEvent → ImGui → Editor::handleEvent()   │
│  │                                                    │
│  │  Editor::update()                                  │
│  │    InputDispatcher::beginFrame()                   │
│  │                                                    │
│  │  Application::render()                             │
│  │    SDL_RenderClear                                 │
│  │    Editor::render()  (canvas + preview)            │
│  │    Editor::renderUI() (ImGui history window)       │
│  │    ImGui::Render + RenderDrawData                  │
│  │    SDL_RenderPresent                               │
│  └────────────────────────────────────────────────────┘
│
└─ SDL_EVENT_QUIT → m_running = false
   ImGui shutdown
   Editor destroyed (all owned objects freed)
   Window destroyed (SDL_Renderer, SDL_Window freed)
   SDL_Quit()
```

Evidence: `App.cpp`

---

## Main Loop

```
App::Application::run()
│
├── SDL_GetTicks()   ← start of frame
│
├── handleEvents()
│   └── while SDL_PollEvent(&m_event)
│       ├── ImGui_ImplSDL3_ProcessEvent()
│       ├── if SDL_EVENT_QUIT → m_running = false
│       ├── if ImGui::GetIO().WantCaptureMouse → continue
│       └── Editor::handleEvent(m_event)
│
├── Editor::update()
│   └── InputDispatcher::beginFrame()   ← resets pressed/released flags
│
└── Application::render()
    ├── ImGui NewFrame setup
    ├── SDL_RenderClear (grey: 128,128,128,255)
    ├── Editor::render()
    │   ├── Renderer::renderTarget(m_canvas, viewport, docTransform)
    │   └── if activeTool->usesPreview():
    │       └── Renderer::renderTarget(m_preview, viewport, docTransform)
    ├── Editor::renderUI()  ← History ImGui window
    ├── ImGui::Render()
    ├── ImGui_ImplSDLRenderer3_RenderDrawData()
    └── SDL_RenderPresent()
```

**deltaTime note:** `float deltaTime = (now - last) / 1000.0f` is computed in
`run()` every frame but is never passed to any function. It is currently dead.

Evidence: `App.cpp`

---

## Coordinate Spaces

Three coordinate spaces are used. Transforms are applied in the Editor before
any coordinate reaches a tool.

```
Screen Space
  (SDL mouse event pixels, top-left origin, not zoom-aware)
       │
       │  Viewport::screenToWorld(screen)
       │  = { (screen.x - pan.x) / zoom,
       │       (screen.y - pan.y) / zoom }
       ▼
World Space
  (zoom-adjusted, pan-adjusted, canvas at docTransform.position)
       │
       │  subtract docTransform.position
       │  (done inside screenToCanvas)
       ▼
Canvas Space
  (pixel coordinates, 0,0 = top-left of canvas surface)
  Tools receive only this space.
```

**All three transform sites in Editor::handleEvent():**

| Event | Location | Transform call |
|---|---|---|
| Mouse down | `Editor.cpp` mousedown block | `viewport.screenToCanvas(mousePos, m_docTransform)` |
| Mouse move | `Editor.cpp` mousemove block | `viewport.screenToCanvas(mousePos, m_docTransform)` |
| Mouse up | `Editor.cpp` mouseup block | `viewport.screenToCanvas(mousePos, m_docTransform)` |

After transform, coordinates are clamped to canvas bounds via
`Editor::clampToCanvas()` before reaching tools.

Evidence: `Editor.cpp`, `Viewport.cpp`, `Viewport.h`

---

## Known Structural Issues

These are verified from source, not from `Improvements.md`.

### 1. Duplicate PreviewLayer

Two `PreviewLayer` instances exist at runtime:

| Instance | Owner | Used by tools | Rendered |
|---|---|---|---|
| `Editor::m_preview` | `Editor` | Yes — passed via `ToolContext` | Yes — `Editor::render()` |
| `Canvas::m_preview` | `Canvas` | No | No |

`Canvas::m_preview` is resized during `Canvas::resize()` via
`m_preview.allocate(w, h)` but is never read, never passed to any tool, and
never rendered.

Evidence: `Canvas.cpp::resize()`, `Editor.cpp::makeToolContext()`,
`Editor.cpp::render()`

### 2. m_boundingBox is Write-Only

`RenderTarget::m_boundingBox` is a public `SDL_Rect` written by Line, Rect,
and Circle tools during every mouse event. No read sites exist in the provided
source. `RenderTarget::updateBounds()` is defined but no call site was found
in tool code; tools write `m_boundingBox` directly.

Evidence: `RenderTarget.h` (declaration), `Line.cpp`, `Rect.cpp`, `Circle.cpp`
(write sites), no read sites found.

### 3. Canvas::m_renderTargets is Unused

`std::vector<RenderTarget*> m_renderTargets` is declared in `Canvas.h` but
never populated or iterated.

Evidence: `Canvas.h` (declaration), `Canvas.cpp` (no usage).

### 4. SnapshotCommand Invalidates Full Canvas on Undo/Redo

`SnapshotCommand::undo()` and `redo()` both call `canvas.markDirty()`, which
marks the entire canvas dirty and causes a full texture upload. `m_bounds` is
available in the command but is not used for targeted invalidation.

Evidence: `SnapshotCommand.h` — `undo()` and `redo()` both end with
`canvas.markDirty()`.

### 5. lockSurface Does Not Detect Failure on SDL3

`Globals.h::lockSurface()` checks `SDL_LockSurface(surface) < 0`. In SDL3,
`SDL_LockSurface` returns `bool` (true = success). `false < 0` evaluates to
`0 < 0 = false`, so a lock failure is silently ignored and the function returns
`true`.

Evidence: `Globals.h::lockSurface()`, SDL3 API (bool return type).

---

## Extension Points

### Adding a New Tool

1. Subclass `StrokeTool` (freehand) or `GeometricTool` (shape with preview).
2. Implement `onMouseDown`, `onMouseMove`, `onMouseUp`.
3. Return a `std::unique_ptr<SnapshotCommand>` from `onMouseUp`; return
   `nullptr` if no canvas change occurred.
4. Override `usesPreview()` to return `true` if the tool renders to
   `ctx.preview` during drag.
5. Register in `Editor::setupTools()` with a string key.
6. Bind a keyboard shortcut in `Editor::setupInputBindings()`.
7. Add a value to the `InputCommand` enum in `InputDispatcher.h`.

No changes to Renderer, Canvas, CommandManager, or Viewport are needed.

### Adding a New Keyboard Shortcut

1. Add a value to `InputCommand` in `Input/InputDispatcher.h`.
2. Bind the scancode in `Editor::setupInputBindings()` via
   `m_input.keyBinds(scancode, InputCommand::VALUE)`.
3. Register a lambda via `m_input.bindActions(InputCommand::VALUE, [this]{...})`.

### Changing the Canvas Size

Call `Editor::resizeCanvas(w, h, policy)`. `ResizePolicy` controls anchor
(`TOPLEFT` or `CENTER`) and fill (`TRANSPARENT` or `BACKGROUNDCOLOR`). The
editor guards against resize during an active tool interaction.

### Adding a Debug Panel

`UI/Console.h` contains standalone `void` functions that call ImGui APIs.
Add a new function there and call it from `Editor::renderUI()` or
`Application::render()`.
