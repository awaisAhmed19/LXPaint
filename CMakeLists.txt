
cmake_minimum_required(VERSION 3.15)

project(LXpaint LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)

add_executable(lxpaint
    src/main.cpp
)

target_link_libraries(lxpaint
    PRIVATE SDL2::SDL2
)

# Linux / macOS usually don't need SDL2main
# Windows might — handle later when needed

