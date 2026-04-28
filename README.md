# LXPAINT

A lightweight paint application built with **SDL3** and **ImGui**, designed to explore **low-level rendering, algorithm behavior, and clean system architecture**.

This is not just a drawing app. It’s a controlled environment for understanding how graphics pipelines and tools actually work.

---

## Features

- Pencil, Line, and Eraser tools
- Undo / Redo via Command Pattern
- Real-time comparison of:
  - Bresenham Line Algorithm
  - DDA (Digital Differential Analyzer)

- Built-in profiler (microsecond precision)
- Fullscreen SDL3 rendering
- ImGui debug console and overlays
- Modular architecture for easy extension

---

## Core Ideas

### Command Pattern (Undo/Redo)

Every drawing operation is stored as a command with:

- A snapshot before execution
- A snapshot after execution

This allows reversible operations without hacks.

---

### CPU → GPU Rendering Flow

Rendering is split cleanly:

- CPU: draw pixels to `SDL_Surface`
- GPU: sync to `SDL_Texture`
- Renderer: display texture

---

### Algorithm Comparison System

Each stroke runs both algorithms:

- Bresenham
- DDA

Execution time is recorded and stored continuously for analysis.

---

### Tool Architecture

Tools are not random logic blobs. They follow a strict lifecycle:

Base Abstraction:

```
onMouseDown → onMouseMove → onMouseUp → Command
```

---

### Event Handling Pipeline

Input is layered:

1. SDL processes raw events
2. ImGui captures UI interactions
3. Tools execute only if UI is not consuming input

---

## Project Structure

```
src/
├── Commands/        # Undo/Redo system
├── Core/            # Canvas, Renderer, Logger, Profiler
├── Tools/           # Pencil, Line, Eraser
├── UI/              # Console and overlays
├── App.*            # Main loop and orchestration
├── Globals.h        # Shared config and utilities
└── main.cpp         # Entry point
```

---

## Controls

| Action    | Key      |
| --------- | -------- |
| Undo      | Ctrl + Z |
| Redo      | Ctrl + Y |
| Pencil    | Ctrl + P |
| Line Tool | Ctrl + L |
| Eraser    | Ctrl + E |

---

## Build & Run

### Requirements

- SDL3
- ImGui (SDL3 + Renderer backend)
- C++20

### Build

```bash
mkdir build
cd build
cmake ..
make
./LXPAINT
```

---

## Why This Project Matters

This project focuses on fundamentals most people skip:

- Manual CPU → GPU data flow
- Real-time algorithm benchmarking
- Proper undo/redo design
- Separation of intent (tools) vs execution (engine)

It’s not about features. It’s about understanding the system.

---

## Future Improvements

- Shape tools (rectangle, circle)
- Flood fill (bucket tool)
- Layers
- Texture brushes
- GPU rendering pipeline
- File save/load support
- Plugin-based tool system

---

## Known Limitations

- No file persistence yet
- CPU rendering may limit scalability
- Undo stack increases memory usage

---

## Entry Point

Start here:

---

## Philosophy

Tools express intent.
Engine enforces rules.
Renderer pushes pixels.
UI only connects everything.

---
