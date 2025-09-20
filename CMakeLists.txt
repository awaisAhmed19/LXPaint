cmake_minimum_required(VERSION 3.10)
project(LXpaint)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 packages
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Source files
file(GLOB SRC_FILES src/*.cpp src/*.h)

# Executable
add_executable(LXpaint ${SRC_FILES})

# Link libraries (modern target-based)
target_link_libraries(LXpaint PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
)

# Run target
add_custom_target(run
    COMMAND LXpaint
    DEPENDS LXpaint
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

