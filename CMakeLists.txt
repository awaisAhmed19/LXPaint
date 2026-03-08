cmake_minimum_required(VERSION 3.15)
project(LXpaint LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Find raylib (must be installed locally) ----
find_package(raylib REQUIRED)

# ---- Add executable ----
add_executable(lxpaint
    src/main.cpp
    # src/canvas.cpp
)

# ---- Link raylib ----
target_link_libraries(lxpaint PRIVATE raylib)
