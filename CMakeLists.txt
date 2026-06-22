cmake_minimum_required(VERSION 3.10)

project(lxpaint VERSION 1.0)


set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
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
)

# C++ Standard
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
