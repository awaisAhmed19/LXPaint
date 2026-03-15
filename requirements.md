LXPaint
│
├── App                          ← owns the game loop, ties everything together
│   ├── InputHandler             ← translates raw mouse/keyboard into events
│   ├── StateManager             ← what tool is active, current color, brush size
│   └── Renderer                 ← draws canvas + UI each frame
│
├── Canvas                       ← owns the RenderTexture2D
│   ├── applyCommand(Command*)   ← executes a draw operation
│   ├── undo()                   ← pops undo stack
│   └── redo()                   ← pops redo stack
│
├── Tools                        ← each tool produces Commands
│   ├── PencilTool               ← produces DrawStrokeCommand
│   ├── FillTool                 ← produces FloodFillCommand
│   ├── ShapeTool                ← produces DrawShapeCommand
│   └── EraserTool               ← produces DrawStrokeCommand (color=WHITE)
│
├── Commands                     ← the actions themselves
│   ├── DrawStrokeCommand
│   ├── FloodFillCommand
│   └── DrawShapeCommand
│
└── UI                           ← toolbar, palette, brush size slider
    ├── Toolbar
    ├── ColorPalette
    └── BrushSizeSlider
