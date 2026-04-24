# LXPaint — CPU-Based Graphics Algorithm Benchmarking Suite

## Introduction

**LXPaint** is a high-performance raster graphics application designed for **academic research and algorithmic analysis**. Unlike conventional painting tools, LXPaint implements a **dual-execution profiling system** that runs competing algorithms simultaneously during user interaction, capturing microsecond-level performance data mapped against geometric magnitude (distance traveled, area filled).

The primary objective is to bridge the gap between **theoretical computational geometry** and **real-world CPU performance characteristics** by measuring:
- **Branch prediction efficiency** (Bresenham vs. DDA)
- **Cache coherency** (scanline flood fill vs. recursive approaches)
- **Memory access patterns** (row-major pixel iteration vs. random access)

All benchmarks run **in-situ** — performance is measured during live drawing, not in synthetic isolation, ensuring results reflect actual workload conditions including L1/L2 cache states, TLB pressure, and instruction pipeline saturation.

---

## Tech Stack

### Core Graphics Pipeline
- **SDL3** (software rendering)
  - Window/input management via event polling (`SDL_EVENT_*`)
  - CPU-side pixel buffer (`SDL_Surface` ARGB8888 format)
  - Texture streaming to GPU (`SDL_UpdateTexture` for display sync)
  - **No hardware acceleration** — all rasterization occurs on the CPU to eliminate GPU scheduler variance

### UI & Profiling Layer
- **Dear ImGui** (immediate-mode GUI)
  - Real-time FPS histogram (`RenderFPSMonitor`)
  - Live algorithm profiler with rolling 100-sample circular buffers (`RenderLiveProfiler`)
  - Scientific comparison benchmark with distance-normalized scatter plots (`RenderComparisonBenchmark`)
  - Audit log console with thread-safe message queueing (`RenderAuditLogs`)

### Architecture Patterns
- **Command Pattern**: All drawing operations encapsulated as `DrawCommand` objects for non-destructive undo/redo
- **Manager Pattern**: `ToolManager` (tool registry), `CommandManager` (history stack)
- **Two-Layer Rendering**:
  - `Canvas::drawingSurface` (permanent CPU-side buffer)
  - `Canvas::previewTexture` (GPU-side overlay for XOR-based line preview)

### Build System
- **CMake** with `find_package(SDL3)` for local library resolution
- **clangd LSP** integration via `compile_commands.json`
- **C++20 strict conformance** (concepts, ranges, `std::format`)

---

## Project Anatomy

```
lxpaint/
├── src/
│   ├── App.h/cpp              # Main application controller (SDL init, event loop, ImGui lifecycle)
│   ├── Globals.h              # Global enums (LineAlgo, FloodFillAlgo), vec2<T> template, color constants
│   ├── main.cpp               # Entry point, instantiates App and starts run loop
│   │
│   ├── core/
│   │   ├── Canvas.h           # Dual-buffer management (CPU surface + GPU textures)
│   │   ├── Command.h          # Abstract base for undoable operations
│   │   ├── CommandManager.h   # Undo/redo stack controller
│   │   ├── Logger.h           # Thread-safe timestamped logging with circular history buffer
│   │   └── Profiler.h         # Race session recorder, per-algorithm stats aggregator
│   │
│   ├── Tools/
│   │   ├── BaseTool.h         # Abstract interface (onMouseDown/Move/Up returning Command*)
│   │   ├── Pencil.h/cpp       # Freehand drawing with Bresenham vs DDA profiling during drag
│   │   ├── Line.h/cpp         # Rubber-band line tool with three optimization modes (brute-force, dirty-rect, XOR overlay)
│   │   ├── FloodFill.h/cpp    # Scanline vs recursive 4-connected fill with tolerance-based color matching
│   │   └── ToolManager.h      # String-keyed tool registry with active tool switching
│   │
│   ├── commands/
│   │   └── DrawCommand.h      # Snapshot-based command storing before/after SDL_Surface states
│   │
│   └── UI/
│       └── Console.h          # ImGui rendering logic for all profiling widgets
│
└── external/
    └── imgui/                 # Dear ImGui library (submodule or vendored)
```

### File Responsibilities

| File | Responsibility |
|------|---------------|
| **App.cpp** | SDL window initialization, event dispatch to active tool, ImGui frame orchestration |
| **Globals.h** | Shared type definitions (vec2 with concept constraints), algorithm selection enums, ARGB color palette |
| **Canvas.h** | Manages CPU-side pixel buffer and GPU texture synchronization via `syncTexture()` |
| **Command.h** | Defines abstract `execute()` and `undo()` interface for reversible operations |
| **CommandManager.h** | Dual-stack manager for undo/redo with automatic redo invalidation on new command |
| **Logger.h** | Thread-safe logging with ANSI color codes and fixed-size history buffer |
| **Profiler.h** | Aggregates per-algorithm timing data, commits race sessions to comparison storage |
| **BaseTool.h** | Enforces tool lifecycle contract (down → move* → up) returning optional Command |
| **Pencil.cpp** | Continuous drawing via Bresenham/DDA with per-segment profiling |
| **Line.cpp** | Three preview strategies: full blit, dirty-rect blit, XOR erase-and-redraw |
| **FloodFill.cpp** | Scanline (stack-based span propagation) vs recursive (4-connected neighbor stack) |
| **ToolManager.h** | Maps string keys to heap-allocated tool instances with active pointer tracking |
| **DrawCommand.h** | Stores deep copies of before/after surfaces, blits appropriate state on execute/undo |
| **Console.h** | ImGui window rendering for logs, FPS graph, algorithm stats, and scientific benchmark plot |

---

## Features & Benchmarks

### Implemented Algorithms

#### Line Rasterization
| Algorithm | Strategy | Branch Profile | Cache Behavior |
|-----------|----------|----------------|----------------|
| **Bresenham** | Integer-only incremental error | Predictable (2 conditionals per pixel) | Sequential row access |
| **DDA** | Floating-point interpolation | Branch-free inner loop | Sequential with rounding overhead |

**Profiling Methodology**: Both algorithms execute on identical input during mouse drag; microsecond deltas recorded per segment and mapped to Euclidean distance traveled.

#### Flood Fill
| Algorithm | Strategy | Stack Depth | Memory Pattern |
|-----------|----------|-------------|----------------|
| **Scanline** | Horizontal span propagation | O(h) where h = region height | Row-major sequential writes |
| **Recursive** | 4-connected neighbor expansion | O(n) where n = pixel count | Random access (cache-hostile) |

**Profiling Methodology**: Both algorithms run on identical seed point; pixel count and execution time recorded. Magnitude measured in **points filled** rather than distance.

### Benchmark Visualization

#### 1. **FPS Monitor** (`RenderFPSMonitor`)
- Circular buffer of 100 frame times
- Live histogram with current FPS and latency readout
- Detects frame pacing issues from algorithm overhead

#### 2. **Live Algorithm Profiler** (`RenderLiveProfiler`)
- Per-function rolling average (last 100 executions)
- Separate sparkline for each algorithm variant
- Useful for identifying performance regression during development

#### 3. **Scientific Comparison Benchmark** (`RenderComparisonBenchmark`)
- **X-axis**: Magnitude (pixels for line, points for fill)
- **Y-axis**: Execution time (microseconds)
- **Multi-run overlay**: Each drawing session plotted as separate polyline
- **Real average calculation**: Displayed in legend (not visual approximation)
- **Auto-scaling axes** with labeled grid divisions

**Legend Colors**:
- **Bresenham**: Orange (`#FF7F00`)
- **DDA**: Blue (`#007FFF`)
- **Scanline Fill**: Green (`#33FF33`)
- **Recursive Fill**: Red (`#FF3333`)

---

## Usage

### Building

```bash
# Install dependencies (Arch Linux)
sudo pacman -S sdl3 cmake clang

# Configure
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build build

# Link compile_commands.json for clangd
ln -sf build/compile_commands.json .
```

### Running

```bash
./build/lxpaint
```

### Controls

| Key Binding | Action |
|-------------|--------|
| **F1** | Toggle fullscreen |
| **Ctrl+Z** | Undo last operation |
| **Ctrl+Y** | Redo |
| **Ctrl+C** | Clear canvas |
| **Ctrl+1** | Switch to Bresenham (line tool) |
| **Ctrl+2** | Switch to DDA (line tool) |
| **Left Click + Drag** | Draw with active tool |

### Interpreting Benchmark Results

1. **During Drawing**: Observe the "Live Algorithm Profiler" — if one algorithm consistently shows lower microsecond values, it has better performance for the current workload.

2. **After Drawing**: Check the "Scientific Comparison Benchmark":
   - **Horizontal spread**: How performance scales with magnitude
   - **Vertical clustering**: Consistency of algorithm (tight cluster = predictable performance)
   - **Slope**: Rate of performance degradation (steeper = worse scaling)

3. **Expected Observations**:
   - **DDA typically faster for short lines** (FPU pipelining)
   - **Bresenham wins on long lines** (integer-only avoids floating-point stalls)
   - **Scanline fill dominates** (2-3 orders of magnitude faster on large regions)

---

## Current Limitations & Planned Work

### Functional but Unoptimized
- ✅ Pencil, Line, FloodFill tools operational
- ✅ Undo/Redo working via Command pattern
- ✅ Real-time profiling with distance normalization

### Planned Features
- ⏳ **Bézier curve tool** (De Casteljau vs forward differencing)
- ⏳ **Lasso selection** (Point-in-polygon via ray casting vs winding number)
- ⏳ **Clipping algorithms** (Cohen-Sutherland, Liang-Barsky)
- ⏳ **Hotkey-based tool switching** (currently hardcoded in `App::App`)
- ⏳ **Brush size control** (UI slider for `brushSize` parameter)
- ⏳ **Color picker** (currently hardcoded in tool constructors)

### Known Issues
- **FloodFill tolerance**: RGB Euclidean distance calculation may over-fill on gradients
- **Line XOR mode**: Color XOR operation ignores alpha channel (cosmetic only)
- **No CMake install target**: Binary must be run from build directory

---

## Academic Context

This project serves as the **implementation component** for a graduate-level research paper on:

> **"Comparative Performance Analysis of Classical Rasterization Algorithms on Modern x86-64 Microarchitectures"**

The benchmarking framework captures:
- **Instruction-level parallelism** (ILP) utilization
- **Branch predictor training effects** on Bresenham's conditionals
- **L1 cache hit rates** for row-major vs scattered pixel writes
- **SIMD underutilization** in scalar implementations (motivates vectorization research)

All data exported from ImGui can be post-processed for:
- Statistical significance testing (t-test for algorithmic superiority)
- Regression analysis (performance vs. input parameters)
- Hardware counter correlation (perf/VTune integration planned)

---

## References

### Primary Literature
1. **Bresenham, J.E.** (1965). "Algorithm for computer control of a digital plotter." *IBM Systems Journal* 4(1): 25–30.
2. **Newman, W.M. & Sproull, R.F.** (1979). *Principles of Interactive Computer Graphics* (2nd ed.). McGraw-Hill. (Scanline Fill, pp. 389-392)
3. **Foley, J.D. et al.** (1995). *Computer Graphics: Principles and Practice* (2nd ed.). Addison-Wesley. (Chapters 3.2–3.4)

### Technical Dependencies
- [SDL3 Documentation](https://wiki.libsdl.org/SDL3/FrontPage)
- [Dear ImGui Repository](https://github.com/ocornut/imgui)
- [C++20 Standard (ISO/IEC 14882:2020)](https://isocpp.org/std/the-standard)

---

## License

**Academic Use Only** — This software is intended for educational and research purposes. Commercial use requires explicit permission from the author.

---

## Contributing

This is a research prototype. For academic collaboration or reproducibility inquiries, contact the author via institutional email.

---

**Version**: 0.2.0-alpha
**Last Updated**: April 2026
**Author**: Awais (CS Postgraduate, Computational Geometry Research)
