
cmake_minimum_required(VERSION 3.10)
project(MSpaint)

set(CMAKE_CXX_STANDARD 17)

# Include all source files
file(GLOB SOURCES src/*.cpp)

# Executable
add_executable(MSpaint ${SOURCES})

# SDL2

find_package(PkgConfig REQUIRED)

# Core SDL2
pkg_check_modules(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})
link_directories(${SDL2_LIBRARY_DIRS})

# SDL2_image
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
link_directories(${SDL2_IMAGE_LIBRARY_DIRS})

# SDL2_ttf
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
include_directories(${SDL2_TTF_INCLUDE_DIRS})
link_directories(${SDL2_TTF_LIBRARY_DIRS})

# SDL2_gfx
pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
include_directories(${SDL2_GFX_INCLUDE_DIRS})
link_directories(${SDL2_GFX_LIBRARY_DIRS})

# Link everything together

target_link_libraries(MSpaint
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_GFX_LIBRARIES}
)
# Include your own headers
target_include_directories(MSpaint PRIVATE include)

