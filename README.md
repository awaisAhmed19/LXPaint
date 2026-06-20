<div align="center">

# 🎨 LXPaint

**A native C++ paint app that's secretly a computational geometry research project wearing a trench coat.**

Built with SDL3 + Dear ImGui. Inspired by the MS Paint of your childhood. Engineered like nobody's watching.

[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=flat-square)](https://en.cppreference.com/w/cpp/20)
[![SDL3](https://img.shields.io/badge/SDL-3.0-orange.svg?style=flat-square)](https://www.libsdl.org/)
[![Dear ImGui](https://img.shields.io/badge/UI-Dear%20ImGui-9cf.svg?style=flat-square)](https://github.com/ocornut/imgui)
[![Status](https://img.shields.io/badge/status-actively%20hacked%20on-brightgreen.svg?style=flat-square)]()
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-ff69b4.svg?style=flat-square)]()

<!-- ![LXPaint screenshot](docs/screenshot.png) -->

</div>

---

## What is this?

LXPaint is a from-scratch raster painting application, built without a game engine, without a canvas library, without shortcuts. Just SDL3, Dear ImGui, and a lot of opinions about how a graphics editor *should* be architected.

It started as a way to answer a deceptively simple question: **what actually happens, pixel by pixel, when you drag a paintbrush across a screen?** Flood fill, Bresenham lines, ellipse rasterization, dirty-rectangle GPU sync, undo/redo as a command stack, viewport math that doesn't fall apart when you resize the window — all of it lives here, readable and pulled apart on purpose.

It also moonlights as the implementation half of a postgraduate research project on computational geometry algorithms. So if the code occasionally reads like it's being profiled, timed, and cross-examined — that's why.

---

## Why it exists

Most paint apps are black boxes. LXPaint is the opposite: every algorithm is hand-rolled and exposed, every subsystem has one job, and the whole thing is built to be *read*, not just used.

This project exists to dig into:

- Raster graphics algorithms (line, ellipse, flood fill — the classics, done properly)
- Dirty-rectangle rendering and partial GPU texture uploads
- Command-pattern undo/redo with bounded memory
- Viewport math: pan, zoom, and screen ↔ canvas coordinate transforms
- Tool interaction as a clean, swappable contract
- What a graphics editor's architecture looks like when responsibility boundaries are taken seriously

Feature completeness is not the goal. Clarity is.

---

## Features

### Drawing tools (fully wired up)

| Tool | What it does |
|---|---|
| ✏️ Pencil | 1px, no mercy |
| 📏 Line | Bresenham under the hood |
| ▭ Rectangle | Stroke, live preview |
| ⬭ Ellipse | Theta-step polygon approximation |
| 🧽 Eraser | Square brush, restores background |
| 🪣 Flood Fill | Iterative stack-based fill (no stack overflows here) |
| 💨 Airbrush | Randomized spray with configurable radius & density |

### On the toolbar, options ready, tool logic incoming

Free-form Select, Rectangle Select, Eyedropper, Magnifier, Brush, Text, Curve, Polygon, Rounded Rectangle — these already have their classic-Paint-style option panels wired up in the UI (shape pickers, line widths, zoom presets, opaque/transparent toggles, the works). The underlying tool *behavior* hasn't landed yet. The toolbox is ahead of the tools — that gap is basically an open invitation.

### Everything around the drawing

- Dirty-rectangle tracking with partial texture uploads — only repainted pixels touch the GPU
- Pan, zoom-to-cursor, and a viewport that survives window resizing without flinching
- Command-pattern undo/redo with bounded depth *and* bounded memory
- A genuinely faithful classic-Paint UI: ribbon, tool grid, 28-color palette, status bar — all hand-drawn with `ImDrawList`, not default ImGui chrome
- Built-in profiling utilities for comparing algorithm implementations head-to-head

---

## Architecture, in one breath

```
Input Layer
    ↓
Viewport Transform (screen → world → canvas)
    ↓
Tool Layer (expresses intent, owns nothing)
    ↓
Canvas (the one true bitmap)
    ↓
Command System (undo/redo)
    ↓
Renderer (CPU rasterize → dirty rect → GPU texture sync)
    ↓
SDL3
```

Every layer has exactly one job. Tools don't know about rendering. The renderer doesn't know what a "stroke" is. The canvas doesn't care who's drawing on it. If you've ever wanted to see that kind of separation actually held to under real feature pressure, the source is right there — start with `Editor.cpp` and follow the data.

Full breakdown — ownership maps, coordinate spaces, object lifetimes, the whole audit trail — lives in [`Documentation/`](Documentation/).

---

## Getting started

### You'll need

- A C++20 compiler
- CMake 3.10+
- SDL3 and SDL3_image (via your package manager or built from source)

### Fastest path

```bash
git clone <repository>
cd LXPaint

./bootstrap.sh   # pulls SDL and Dear ImGui into external/
./build.sh       # configures, builds, and launches LXPaint
```

### The manual way

```bash
mkdir build && cd build
cmake ..
cmake --build . --parallel
./lxpaint
```

---

## Controls

| Action | Shortcut |
|---|---|
| Switch tool | Click the toolbar |
| Undo | `Ctrl + Z` |
| Redo | `Ctrl + Y` |
| Zoom In / Out | Mouse Wheel |
| Pan | `Space` + Drag |
| Grow Canvas | `Ctrl` + `=` |
| Shrink Canvas | `Ctrl` + `-` |

---

## Project layout

```
src/
├── App/          window, bootstrap, global types
├── Document/     Canvas, RenderTarget, PreviewLayer — the bitmap layer
├── Editor/
│   ├── Commands/     undo/redo command stack
│   ├── Interaction/  tool context & interaction state
│   ├── Tools/        every tool, one file each
│   └── Viewport/     pan/zoom/coordinate transforms
├── Input/        SDL events → typed engine state
├── Rendering/    Rasterizer (CPU pixels) + Renderer (GPU sync)
├── Systems/      logging, assertions, profiling
└── UI/           ImGui-based classic-Paint interface

Documentation/    architecture, audit, and roadmap docs — the project's own paper trail
```

---

## Roadmap

The honest, unranked list of what's next:

- Wire up the tools that already have a toolbar UI waiting for them (Selection, Text, Brush, Curve, Polygon, Magnifier)
- A real layer system (the placeholder is already sitting in `Canvas`, just waiting)
- Source-clipped rendering so zoomed-out canvases stop uploading pixels nobody can see
- A `.LXP` save format that doesn't try to serialize an `SDL_Texture*` (we know better now)
- Headless testing, once the SDL dependency is abstracted out of the rasterizer

Check [`Documentation/Roadmap.md`](Documentation/Roadmap.md) for the fully itemized, audit-backed version.

---

## Contributing

Before touching core systems, it's worth skimming:

1. Dirty-rectangle propagation
2. The command lifecycle (`SnapshotCommand`, `CommandManager`)
3. Viewport coordinate transforms (screen → world → canvas)
4. The tool interaction contract (`BaseTool` vs `ClickTool`)
5. Renderer sync timing

Adding a new tool is the easiest way in — subclass `StrokeTool` or `GeometricTool`, implement three mouse callbacks, register it, done. See [`Documentation/Architecture.md`](Documentation/Architecture.md#adding-a-new-tool) for the exact steps.

Found a bug? `Documentation/Audit.md` already tracks a few known ones with severity and evidence — feel free to add to the list or, better, cross one off.

---

## Philosophy

> Tools express intent. Engine enforces rules. Renderer pushes pixels. UI connects everything.

LXPaint favors explicit architecture over convenience abstractions. It's not trying to be Photoshop. It's trying to be a paint app you could fully explain, end to end, on a whiteboard.

---

## Acknowledgments

Built on [SDL3](https://www.libsdl.org/) and [Dear ImGui](https://github.com/ocornut/imgui) — without which this would just be a very ambitious `malloc` call.

If LXPaint taught you something about how graphics editors work, a ⭐ goes a long way.
