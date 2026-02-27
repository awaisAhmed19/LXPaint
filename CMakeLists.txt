cmake_minimum_required(VERSION 3.15)
project(LXpaint LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Try config package first; fall back to module if you prefer:
# find_package(SDL2 CONFIG REQUIRED)  # Uncomment if you know you have the config package
find_package(SDL2 REQUIRED)

add_executable(lxpaint
    src/main.cpp
    src/canvas.cpp
)

# Always link the core SDL2 library
target_link_libraries(lxpaint PRIVATE SDL2::SDL2)

# Windows-specific: link SDL2main if available and make it a GUI app
if (WIN32)
    if (TARGET SDL2::SDL2main)
        target_link_libraries(lxpaint PRIVATE SDL2::SDL2main)
    endif()
    set_target_properties(lxpaint PROPERTIES WIN32_EXECUTABLE TRUE)
endif()