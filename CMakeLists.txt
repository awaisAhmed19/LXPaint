cmake_minimum_required(VERSION 3.10)
project(lxpaint VERSION 1.0)

# 1. Configure SDL options before adding the directory
# We disable tests and documentation to speed up build time
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

# 2. Add the SDL subdirectory
add_subdirectory(external/SDL)

# 3. Define your executable
add_executable(${PROJECT_NAME} src/main.cpp)

# 4. Link SDL.
# Note: SDL3 uses "SDL3::SDL3", while SDL2 uses "SDL2::SDL2"
# This template assumes you cloned the latest (SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
