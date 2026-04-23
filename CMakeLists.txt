cmake_minimum_required(VERSION 3.10)
project(lxpaint VERSION 1.0)

# 1. SDL3 Configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL)

# 2. Collect Project Sources (recursive handles src/core, src/Tools, etc.)
file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

# 3. Define ImGui Sources
set(IMGUI_SRCS
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_sdl3.cpp
    external/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

# 4. Create Single Executable
# We combine PROJECT_SOURCES and IMGUI_SRCS here
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${IMGUI_SRCS})

# 5. Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/Tools
    src/core
    src/UI
    external/imgui
    external/imgui/backends
)

# 6. Linking
# Ensure we link against the SDL3 target created by add_subdirectory
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# 7. C++ Standard
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
