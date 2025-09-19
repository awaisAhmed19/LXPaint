# LXPaint

**LXPaint** is a lightweight paint application for Linux, written in **C++** and powered by **SDL2**.  
It provides a simple, fast, and flexible canvas for creating sketches, pixel art, and other graphics.

---

## Features

- Basic drawing tools: pencil, eraser, brush
- Color selection
- Undo/redo support
- SDL2-based window and rendering
- Fast and responsive on low-end hardware
- Cross-platform potential (Linux-focused currently)

---

## Requirements

- Linux (tested on Arch Linux / Ubuntu)
- C++17 or higher
- SDL2 development libraries
- CMake (for building)

### Install SDL2 on Arch Linux:

```bash
sudo pacman -S sdl2 sdl2_image sdl2_ttf
```
### Install SDL2 on Ubuntu/Debian:

```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

## Building
### Clone the repository:

```bash
git clone https://github.com/awaisAhmed19/LXPaint.git
cd LXPaint
mkdir build && cd build
cmake ..
make
```

### Run the application:

```bash
./myapp
(Replace myapp with your actual executable name if different.)
```

## Usage
- Launch the app and use your mouse to draw
- Basic keyboard shortcuts and controls will be added in future releases
- Rendering is handled using SDL2

## Contributing
Contributions are welcome! Feel free to:

- Report bugs
- Suggest features
- Submit pull requests
- Please make sure your code is clean and well-documented.


## Future Plans
- Implement more drawing tools (lines, shapes, fill)
- Add file save/load (PNG, BMP)
- Add layers support
- Improve performance and add undo/redo stack
