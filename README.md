# LXPaint

A lightweight paint application built with **SDL3** and **ImGui**, created to explore how modern graphics editors work internally.

LXPaint is not intended to compete with professional art software. Its primary goal is educational: understanding rendering pipelines, rasterization algorithms, viewport systems, undo/redo architectures, and graphics editor design through a real-world implementation.

---

## Goals

LXPaint exists to explore:

* Raster graphics algorithms
* Dirty rectangle rendering optimization
* GPU texture synchronization
* Undo/redo architectures
* Tool interaction systems
* Viewport and coordinate transforms
* Clean graphics editor architecture
* Modern C++ design patterns

The project prioritizes clarity, maintainability, and architectural experimentation over feature completeness.

---

## Features

### Drawing Tools

* Pencil
* Line
* Rectangle
* Circle
* Eraser

### Rendering Features

* Dirty rectangle tracking
* Partial texture uploads
* Interactive preview rendering
* Zoom and pan support
* Viewport-aware input handling

### Undo / Redo

* Command pattern architecture
* Snapshot-based undo/redo
* Region-based restoration
* Bounded command history

### Developer Tooling

* Integrated profiling utilities
* Debug logging
* FPS monitoring
* Performance instrumentation

---

## Architecture Overview

LXPaint is organized around a strict separation of responsibilities.

```text
Input Layer
    ↓
Viewport Transform System
    ↓
Tool Layer
    ↓
Canvas (Document State)
    ↓
Command System
    ↓
Renderer
    ↓
SDL3
```

### Core Principles

#### Tools Express Intent

Tools do not own rendering logic or undo history.

A tool simply expresses what the user wants to do.

Examples:

* Draw a line
* Draw a rectangle
* Erase pixels
* Paint a stroke

#### Canvas Owns Document State

The canvas is the authoritative bitmap.

Responsibilities:

* Pixel storage
* Canvas dimensions
* Dirty region tracking

The canvas does not manage user interaction or viewport behavior.

#### Commands Make Operations Reversible

Every committed drawing action becomes a command.

This enables:

* Undo
* Redo
* History management

#### Renderer Synchronizes GPU State

The renderer is responsible for:

* Texture synchronization
* Viewport-aware rendering
* GPU presentation

---

## Rendering Pipeline

LXPaint uses a CPU-first rendering model.

```text
Tool
    ↓
Rasterizer
    ↓
Canvas Surface
    ↓
Dirty Region Tracking
    ↓
Texture Synchronization
    ↓
GPU Rendering
```

Only modified regions are uploaded to the GPU whenever possible.

This significantly reduces texture upload bandwidth during interactive drawing.

---

## Viewport System

The viewport provides coordinate transformation between screen space and canvas space.

```text
Screen Space
    ↓
Viewport Transform
    ↓
Canvas Space
```

Features:

* Zoom
* Pan
* Coordinate conversion
* Visible region calculations

Tools operate exclusively in canvas coordinates and remain independent from viewport implementation details.

---

## Undo / Redo System

LXPaint uses a command-based architecture.

```text
Mouse Input
    ↓
Tool Interaction
    ↓
Command Creation
    ↓
Command Execution
    ↓
Undo Stack
```

Commands store only the affected region of the canvas rather than full-canvas snapshots whenever possible.

This keeps history operations efficient and predictable.

---

## Project Structure

```text
src/
├── App/
├── Document/
├── Editor/
│   ├── Commands/
│   ├── Input/
│   ├── Interaction/
│   ├── Tools/
│   └── Viewport/
├── Rendering/
├── Systems/
└── UI/
```

### Major Subsystems

| Directory | Responsibility                        |
| --------- | ------------------------------------- |
| App       | Application startup and main loop     |
| Document  | Canvas and bitmap state               |
| Editor    | User interaction orchestration        |
| Commands  | Undo/redo infrastructure              |
| Tools     | Drawing behavior                      |
| Rendering | Rasterization and GPU synchronization |
| Systems   | Logging, profiling, utilities         |
| UI        | ImGui-based interface                 |

---

## Controls

| Action    | Shortcut         |
| --------- | ---------------- |
| Pencil    | P                |
| Line      | L                |
| Rectangle | R                |
| Circle    | C                |
| Eraser    | E                |
| Undo      | Ctrl + Z         |
| Redo      | Ctrl + Y         |
| Zoom In   | Mouse Wheel Up   |
| Zoom Out  | Mouse Wheel Down |
| Pan       | Space + Drag     |

---

## Building

### Requirements

* C++20
* SDL3
* ImGui
* CMake 3.10+

### Build

```bash
git clone <repository>
cd LXPaint

mkdir build
cd build

cmake ..
cmake --build .

./lxpaint
```

---

## Documentation

Additional documentation can be found in the `docs/` directory.

Suggested structure:

```text
docs/
├── ARCHITECTURE.md
├── COMMAND_SYSTEM.md
├── RENDERING.md
├── VIEWPORT.md
├── ROADMAP.md
└── AUDIT.md
```

---

## Philosophy

> Tools express intent. Engine enforces rules. Renderer pushes pixels. UI connects everything.

Each subsystem should have a single responsibility and clear ownership boundaries.

The project favors explicit architecture over convenience abstractions and serves as a playground for understanding how graphics software is built from first principles.

---

## Contributing

Before modifying core systems, familiarize yourself with:

1. Dirty rectangle propagation
2. Command lifecycle
3. Viewport coordinate transforms
4. Tool interaction contracts
5. Rendering synchronization

