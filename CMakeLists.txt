cmake_minimum_required(VERSION 3.10)
project(lxpaint VERSION 1.0)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
# Project Sources
set(PROJECT_SOURCES
    src/App/main.cpp
    src/App/Window.cpp
    src/App/App.cpp
    #src/UI/Theme.cpp
    src/Input/InputDispatcher.cpp
    src/Editor/Viewport/Viewport.cpp
    src/Editor/Editor.cpp
    src/Rendering/Rasterizer.cpp
    src/Rendering/Renderer.cpp
    src/Document/Canvas.cpp
    src/Document/RenderTarget.cpp
    src/Document/PreviewLayer.cpp
    # Tools
    src/Editor/Tools/SelectionTool.cpp
    src/Editor/Tools/Pencil.cpp
    src/Editor/Tools/FloodFill.cpp
    src/Editor/Tools/Line.cpp
    src/Editor/Tools/Eraser.cpp
    src/Editor/Tools/Rect.cpp
    src/Editor/Tools/Circle.cpp
    src/Editor/Tools/AirBrush.cpp
    src/Editor/Tools/Lasso.cpp
    src/Editor/Tools/RectSelection.cpp
    src/Editor/Tools/CurveLine.cpp
    src/Editor/Tools/RoundedRect.cpp
    src/Editor/Tools/Magnifier.cpp
    src/Editor/Tools/Brush.cpp
    src/Editor/Tools/EyeDropper.cpp
    src/Editor/Tools/Polygon.cpp
    src/Editor/Tools/Text.cpp
    src/UI/Ribbon.cpp
    src/UI/Toolbar.cpp
    src/UI/ColorPalette.cpp
    src/UI/Footer.cpp
    src/UI/LayoutEngine/LayoutEngine.cpp
  )
# ImGui
set(IMGUI_SRCS
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_sdl3.cpp
    external/imgui/backends/imgui_impl_sdlrenderer3.cpp
)
# Executable
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${IMGUI_SRCS}
)
# Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    external/imgui
    external/imgui/backends
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  SDL3::SDL3
  SDL3_image::SDL3_image
  SDL3_ttf::SDL3_ttf
)
# C++ Standard
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Fonts — Text tool calls Text::initFontSystem("../fonts/default.ttf", ...)
# from App::Application's constructor (see App.cpp changes). That relative
# path matches the SAME existing convention Toolbar::init() already uses
# for "../tools_icons/...png": both assume the binary's working directory
# at runtime is one level below the repo root, where tools_icons/ already
# lives. So fonts/ needs to sit at the repo root too (alongside
# tools_icons/), NOT inside the build directory — copying it into
# CMAKE_BINARY_DIR would put it at the wrong relative depth from "../fonts/"
# once the binary runs from inside that build dir.
#
# No copy step is added here, deliberately: place an actual fonts/
# directory containing default.ttf at the repo root yourself (same level
# as tools_icons/), matching how tools_icons/ is already just checked into
# (or placed alongside) the repo rather than something CMake stages.
