# LXPaint — Command System

---

## Table of Contents

1. [Overview](#overview)
2. [Class Hierarchy](#class-hierarchy)
3. [Ownership and Lifetime](#ownership-and-lifetime)
4. [SnapshotCommand — Two Construction Paths](#snapshotcommand--two-construction-paths)
5. [CommandManager](#commandmanager)
6. [Tool Integration](#tool-integration)
7. [Runtime Flow — Stroke Tool](#runtime-flow--stroke-tool)
8. [Runtime Flow — Geometric Tool](#runtime-flow--geometric-tool)
9. [Runtime Flow — Undo](#runtime-flow--undo)
10. [Runtime Flow — Redo](#runtime-flow--redo)
11. [Memory Management](#memory-management)
12. [Known Limitations](#known-limitations)
13. [Extension Points](#extension-points)

---

## Overview

LXPaint uses the Command pattern to implement undo and redo. Every canvas
mutation that a tool makes is captured as a `SnapshotCommand`: a before-image
and an after-image of the affected rectangular region. The command is created
by the tool on mouse-up and pushed onto a `CommandManager` owned by the
`Editor`.

The system has two variants of `SnapshotCommand` construction, corresponding
to two categories of tool:

- **Stroke tools** (Pencil, Eraser): draw continuously during drag, capture a
  full-canvas duplicate at mouse-down and extract the affected region at
  mouse-up.
- **Geometric tools** (Line, Rect, Circle): draw only on mouse-up, capture
  before immediately using the known bounds, then capture after.

---

## Class Hierarchy

```
Command  (abstract)
│   + undo(Canvas&) = 0
│   + redo(Canvas&) = 0
│   + memoryUsage() const = 0
└── SnapshotCommand
        m_bounds:   SDL_Rect
        m_before:   UniqueSurface
        m_after:    UniqueSurface
```

```
BaseTool  (abstract)
│
├── StrokeTool  (Pencil, Eraser)
│       m_backupSurface:  SDL_Surface* (raw, owned, freed on up/deactivate)
│       m_strokeBounds:   SDL_Rect
│
└── GeometricTool  (Line, Rect, Circle)
        (empty intermediate — no added members currently)
```

Source: `Command.h`, `SnapshotCommand.h`, `BaseTool.h`, `StrokeTool.h`,
`GeometricTool.h`

---

## Ownership and Lifetime

```
Tool (value in ToolManager::registry)
  │
  │ creates on mouse-up
  │
  ▼
unique_ptr<SnapshotCommand>
  │
  │ Editor::handleEvent() calls pushCommand()
  │
  ▼
CommandManager::m_undoStack  (deque<CommandEntry>)
  │
  │ undo() moves entry to:
  ▼
CommandManager::m_redoStack  (deque<CommandEntry>)
```

- Commands are created by tools and immediately transferred to `CommandManager`
  via `std::move`.
- `CommandManager` is the sole long-term owner.
- Popping from the front of the undo stack (trim) destroys the command and its
  surfaces.

Source: `CommandManager.h`, `Editor.cpp::handleEvent()` mouse-up block

---

## SnapshotCommand — Two Construction Paths

### Path A: Geometric Tool Constructor

```cpp
SnapshotCommand(SDL_Surface* canvasSurface, SDL_Rect bounds)
```

Used by: Line, Rect, Circle

- Called **before** drawing to canvas.
- `m_before` is captured immediately via `copyRegion()`.
- `m_after` is null until `captureAfter()` is called.
- `captureAfter(SDL_Surface*)` must be called after the rasterizer writes to
  the canvas surface, before the command is returned.

Sequence (from `Line.cpp::onMouseUp`):
```
1. computeLineBounds()
2. SnapshotCommand(canvas->getSurface(), bounds)   ← captures before
3. Rasterizer::bresenham(... canvas->getSurface() ...)
4. canvas->markDirty()
5. m_command->captureAfter(canvas->getSurface())   ← captures after
6. return std::move(m_command)
```

### Path B: Stroke Tool Constructor

```cpp
SnapshotCommand(UniqueSurface before, UniqueSurface after, SDL_Rect bounds)
```

Used by: Pencil, Eraser

- Called **entirely after** the stroke is finished.
- `before` = `copyRegion()` applied to the **backup duplicate** taken at
  mouse-down.
- `after` = `copyRegion()` applied to the **current canvas** at mouse-up.
- Bounds are accumulated across all mouse-move events via `expandStrokeBounds()`.

Sequence (from `Pencil.cpp`):
```
onMouseDown:
  m_backupSurface = SDL_DuplicateSurface(canvas)   ← full copy

onMouseMove (each call):
  expandStrokeBounds(pos, brushSize, ...)
  Rasterizer::bresenham(m_last, pos, canvas, ...)
  canvas->invalidateRect(segmentRect)
  m_last = pos

onMouseUp:
  expandStrokeBounds(pos, ...)
  before = copyRegion(m_backupSurface, m_strokeBounds)
  after  = copyRegion(canvas->getSurface(), m_strokeBounds)
  cmd    = SnapshotCommand(move(before), move(after), m_strokeBounds)
  freeBackupSurface()
  return cmd
```

Source: `SnapshotCommand.h`, `Pencil.cpp`, `Eraser.cpp`, `Line.cpp`,
`Rect.cpp`, `Circle.cpp`

---

## CommandManager

```
CommandManager
  m_undoStack : deque<CommandEntry>
  m_redoStack : deque<CommandEntry>
  m_maxUndoDepth  : size_t  (default 50)
  m_maxMemoryBytes: size_t  (default 256 MB)
```

### pushCommand()

```
pushCommand(unique_ptr<Command> cmd, string description)
  │
  ├── clearRedo()          ← any existing redo history is discarded
  ├── emplace_back to m_undoStack
  └── trimUndoStack()
      └── while size > maxDepth OR memory > maxBytes:
          └── pop_front   ← oldest entry destroyed
```

### undo()

```
undo(Canvas& canvas)
  ├── if m_undoStack empty → return false
  ├── move back() out of m_undoStack
  ├── entry.command->undo(canvas)
  └── push_back to m_redoStack
```

### redo()

```
redo(Canvas& canvas)
  ├── if m_redoStack empty → return false
  ├── move back() out of m_redoStack
  ├── entry.command->redo(canvas)
  └── push_back to m_undoStack
```

### Memory Accounting

`totalMemoryUsage()` iterates both stacks and sums `command->memoryUsage()`.

`SnapshotCommand::memoryUsage()` returns:
```
(m_before ? m_before->w * m_before->h * 4 : 0)
+ (m_after  ? m_after->w  * m_after->h  * 4 : 0)
```

This estimates ARGB8888 pixel cost only. It does not account for SDL surface
headers or alignment padding.

Source: `CommandManager.h`, `SnapshotCommand.h`

---

## Tool Integration

Each tool's `onMouseUp()` returns `std::unique_ptr<Command>`. The Editor
handles the transfer:

```cpp
// Editor.cpp — mouse-up block
std::unique_ptr<Command> command = tool->onMouseUp(mousePos, ctx);
if (command) {
    m_commands.pushCommand(std::move(command), "Draw Stroke");
}
```

Tools return `nullptr` when no canvas change occurred (e.g., mouse-up without
active interaction, failed surface duplication).

Source: `Editor.cpp`, `BaseTool.h`

---

## Runtime Flow — Stroke Tool

```
User presses mouse button
│
└── Editor::handleEvent()  [mouse-down]
    └── Pencil::onMouseDown(pos, ctx)
        ├── beginStrokeBounds(pos, brushSize, w, h)
        ├── freeBackupSurface()
        ├── m_backupSurface = SDL_DuplicateSurface(canvas->getSurface())
        └── Rasterizer::bresenham(pos, pos, canvas, color, brushSize, false)

User moves mouse (repeated)
│
└── Editor::handleEvent()  [mouse-move, leftMouseDown && interaction.active]
    └── Pencil::onMouseMove(pos, ctx)
        ├── expandStrokeBounds(pos, brushSize, w, h)
        ├── Rasterizer::bresenham(m_last, pos, canvas, color, brushSize, false)
        ├── canvas->invalidateRect(segmentRect)
        └── m_last = pos

User releases mouse button
│
└── Editor::handleEvent()  [mouse-up]
    └── Pencil::onMouseUp(pos, ctx)
        ├── expandStrokeBounds(pos, brushSize, w, h)
        ├── before = SnapshotCommand::copyRegion(m_backupSurface, m_strokeBounds)
        ├── after  = SnapshotCommand::copyRegion(canvas->getSurface(), m_strokeBounds)
        ├── cmd    = new SnapshotCommand(move(before), move(after), m_strokeBounds)
        ├── freeBackupSurface()
        └── return cmd
    └── Editor pushes cmd to CommandManager
```

---

## Runtime Flow — Geometric Tool

```
User presses mouse button
│
└── Editor::handleEvent()  [mouse-down]
    └── Line::onMouseDown(pos, ctx)
        ├── interaction->active = true
        ├── m_start = pos
        └── canvas->m_boundingBox = computeLineBounds(...)  [write-only — see audit]

User moves mouse (repeated)
│
└── Editor::handleEvent()  [mouse-move]
    └── Line::onMouseMove(pos, ctx)
        ├── canvas->m_boundingBox = computeLineBounds(...)  [write-only]
        ├── preview->clearRGBA(0, 0, 0, 0)                 [full clear]
        ├── Rasterizer::bresenham(m_start, pos, preview->getSurface(), ...)
        └── preview->markDirty()

User releases mouse button
│
└── Editor::handleEvent()  [mouse-up]
    └── Line::onMouseUp(pos, ctx)
        ├── preview->clearRGBA(0, 0, 0, 0)
        ├── preview->markDirty()
        ├── canvas->m_boundingBox = computeLineBounds(...)  [write-only]
        ├── m_command = new SnapshotCommand(canvas->getSurface(), bounds)  ← before
        ├── Rasterizer::bresenham(m_start, pos, canvas->getSurface(), ...)
        ├── canvas->markDirty()
        ├── m_command->captureAfter(canvas->getSurface())  ← after
        └── return move(m_command)
    └── Editor pushes cmd to CommandManager
```

---

## Runtime Flow — Undo

```
User presses Ctrl+Z
│
└── InputDispatcher fires UNDO action
    └── Editor lambda: m_commands.undo(m_canvas)
        └── CommandManager::undo(canvas)
            ├── entry = move(m_undoStack.back())
            ├── m_undoStack.pop_back()
            ├── entry.command->undo(canvas)
            │   └── SnapshotCommand::undo(canvas)
            │       ├── restoreRegion(canvas.getSurface(), m_before.get(), m_bounds)
            │       │   └── SDL_BlitSurface(m_before, nullptr, canvas_surface, &dstRect)
            │       └── canvas.markDirty()   ← full canvas invalidated (see limitations)
            └── push entry to m_redoStack
```

---

## Runtime Flow — Redo

Identical to undo with stacks reversed:

```
Ctrl+Y → CommandManager::redo()
  ├── entry = move(m_redoStack.back())
  ├── m_redoStack.pop_back()
  ├── entry.command->redo(canvas)
  │   └── SnapshotCommand::redo(canvas)
  │       ├── restoreRegion(canvas, m_after, m_bounds)
  │       └── canvas.markDirty()   ← full canvas invalidated
  └── push entry to m_undoStack
```

---

## Memory Management

### Surface Lifecycle in SnapshotCommand

```
Geometric path:
  copyRegion() → SDL_CreateSurface + SDL_BlitSurface → UniqueSurface
  captureAfter() → copyRegion() → UniqueSurface
  SnapshotCommand destroyed → SDL_Surface_Deleter runs → SDL_DestroySurface

Stroke path:
  mouse-down: SDL_DuplicateSurface → raw ptr in StrokeTool::m_backupSurface
  mouse-up:   copyRegion(backup, bounds) → UniqueSurface (before)
              copyRegion(canvas, bounds) → UniqueSurface (after)
              freeBackupSurface() → SDL_DestroySurface(m_backupSurface)
              SnapshotCommand takes ownership of before+after
```

`UniqueSurface` is `std::unique_ptr<SDL_Surface, SDL_Surface_Deleter>`.
`SDL_Surface_Deleter::operator()` calls `SDL_DestroySurface`.

Source: `SnapshotCommand.h`, `StrokeTool.h`

### Stack Trimming

When `pushCommand()` is called, `trimUndoStack()` runs:
```cpp
while ((m_undoStack.size() > m_maxUndoDepth ||
        totalMemoryUsage() > m_maxMemoryBytes) &&
       !m_undoStack.empty()) {
    m_undoStack.pop_front();   // destroys oldest command + its surfaces
}
```

The check is re-evaluated after each pop, so multiple entries may be removed
in one trim pass.

Source: `CommandManager.h`

---

## Known Limitations

### Full Canvas Invalidation on Undo/Redo (Verified)

`SnapshotCommand::undo()` and `redo()` both call `canvas.markDirty()`.

`markDirty()` sets the dirty rect to the full canvas (`{0, 0, m_width,
m_height}`), causing a full texture upload to the GPU on the next frame even
when the restored region is small.

The command already holds `m_bounds`, which is the exact affected region.
`canvas.invalidateRect(m_bounds)` would upload only the changed pixels.

Evidence: `SnapshotCommand.h` lines 89–95, `RenderTarget.cpp::markDirty()`

### Full Preview Clear on Each Mouse Move (Verified)

Line, Rect, and Circle call `ctx.preview->clearRGBA(0, 0, 0, 0)` on every
`onMouseMove`. This clears the entire preview surface regardless of how much
of it changed.

Evidence: `Line.cpp:39`, `Rect.cpp:43`, `Circle.cpp:38`

### Stroke Tools Duplicate the Entire Canvas at Mouse-Down (Verified)

`Pencil::onMouseDown` and `Eraser::onMouseDown` both call
`SDL_DuplicateSurface(ctx.canvas->getSurface())`. On an 800×550 canvas in
ARGB8888 this allocates ~1.7 MB per stroke start, regardless of stroke size.

Evidence: `Pencil.cpp::onMouseDown`, `Eraser.cpp::onMouseDown`

---

## Extension Points

### Implementing a New Command Type

1. Subclass `Command` in `src/Editor/Commands/`.
2. Implement `undo(Canvas&)`, `redo(Canvas&)`, `memoryUsage() const`.
3. Return an instance from a tool's `onMouseUp()`.

### Implementing a Non-Snapshot Command

`SnapshotCommand` is the only concrete command. If an operation does not fit
the before/after pixel model (e.g., canvas resize, property change), a new
`Command` subclass can hold whatever state is needed. The `CommandManager` is
not coupled to `SnapshotCommand`.

### Changing Undo Depth or Memory Limit

`CommandManager` constructor:
```cpp
explicit CommandManager(
    size_t maxUndoDepth = DEFAULT_MAX_UNDO,    // 50
    size_t maxMemoryMB  = DEFAULT_MAX_MEMORY_MB // 256
);
```

The Editor constructs it as `m_commands(50, 256)` in `Editor.cpp`.
