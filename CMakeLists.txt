cmake_minimum_required(VERSION 3.10)
project(lxpaint VERSION 1.0)

# 1. Configure SDL options
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

# 2. Add SDL Subdirectory
# Ensure the path external/SDL exists and contains SDL's CMakeLists.txt
add_subdirectory(external/SDL)

# 3. Collect all source files (.cpp) automatically
# This searches through src and all subfolders (like Tools)
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 4. Define the executable with all collected sources
add_executable(${PROJECT_NAME} ${SOURCES})

# 5. Set Include Directories
# This allows you to use #include "Canvas.h" or #include "Pencil.h"
# without needing long relative paths like "../../Canvas.h"
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/Tools
)

# 6. Link SDL3
# We use SDL3-shared by default; use SDL3::SDL3-static if you prefer static linking
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# 7. C++ Standard
# Ensure we are using at least C++17 for modern features
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
