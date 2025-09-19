cmake_minimum_required(VERSION 3.10)
project(LXpaint)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} include)

file(GLOB SRC_FILES src/*.cpp)

add_executable(myapp ${SRC_FILES})

# Link SDL2 libraries
target_link_libraries(myapp ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})

add_custom_target(run
    COMMAND myapp
    DEPENDS myapp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
