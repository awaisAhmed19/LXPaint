cmake_minimum_required(VERSION 3.16)
project(Paintlxpaint)

set(CMAKE_CXX_STANDARD 17)

# GLFW
add_subdirectory(external/glfw)

# GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# ImGui
file(GLOB IMGUI_SRC
    external/imgui/*.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui ${IMGUI_SRC})

target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
)

target_link_libraries(imgui PUBLIC glfw)

# Executable
add_executable(lxpaint
    src/main.cpp
    src/renderer/shader.cpp
    src/renderer/quad.cpp
)
target_link_libraries(lxpaint glfw glad imgui)

# Linux fix (important)
if(UNIX)
    target_link_libraries(lxpaint GL dl pthread)
endif()
