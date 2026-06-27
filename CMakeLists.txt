cmake_minimum_required(VERSION 3.21)

project(
    LXPaint
    VERSION 1.0.0
    DESCRIPTION "Classic raster graphics editor built with SDL3 and ImGui"
    LANGUAGES CXX
)

# =============================================================================
# Global settings
# =============================================================================

# Default to Debug when nothing is specified.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Build type not set — defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    set_property(
        CACHE CMAKE_BUILD_TYPE
        PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel"
    )
endif()

# Helps IDEs and clangd find the right files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Dependencies — system libraries discovered via pkg-config / find_package
# =============================================================================

find_package(PkgConfig REQUIRED)

# SDL3 -------------------------------------------------------------------------
find_package(SDL3 REQUIRED CONFIG) # modern SDL3 cmake config
# falls back below if absent
if(NOT TARGET SDL3::SDL3)
    # Older SDL3 installs may not ship a cmake config; use pkg-config.
    pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
    add_library(SDL3::SDL3 ALIAS PkgConfig::SDL3)
endif()

# SDL3_image -------------------------------------------------------------------
# Arch ships the pc file as "sdl3-image"; Debian/Fedora use "SDL3_image". Try
# the cmake config first, then fall back through both pc names.
find_package(SDL3_image QUIET CONFIG)
if(NOT TARGET SDL3_image::SDL3_image)
    pkg_check_modules(SDL3_image QUIET IMPORTED_TARGET sdl3-image)
    if(NOT SDL3_image_FOUND)
        pkg_check_modules(SDL3_image REQUIRED IMPORTED_TARGET SDL3_image)
    endif()
    add_library(SDL3_image::SDL3_image ALIAS PkgConfig::SDL3_image)
endif()

# SDL3_ttf ---------------------------------------------------------------------
# Same naming split: "sdl3-ttf" on Arch, "SDL3_ttf" elsewhere.
find_package(SDL3_ttf QUIET CONFIG)
if(NOT TARGET SDL3_ttf::SDL3_ttf)
    pkg_check_modules(SDL3_ttf QUIET IMPORTED_TARGET sdl3-ttf)
    if(NOT SDL3_ttf_FOUND)
        pkg_check_modules(SDL3_ttf REQUIRED IMPORTED_TARGET SDL3_ttf)
    endif()
    add_library(SDL3_ttf::SDL3_ttf ALIAS PkgConfig::SDL3_ttf)
endif()

# =============================================================================
# Vendored / header-only libraries
# =============================================================================

# ImGui — vendored source
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp"
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC "${IMGUI_DIR}" "${IMGUI_DIR}/backends")
target_link_libraries(imgui PUBLIC SDL3::SDL3)
target_compile_features(imgui PRIVATE cxx_std_17)

# stb — header-only, exposed as an interface library. Header lives at
# third_party/stb_image.h in this project.
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_SOURCE_DIR}/third_party")

# =============================================================================
# Collect application sources
# =============================================================================

# Glob is intentional here: the file list grows with every new tool/dialog. Any
# CMake-aware IDE will re-run cmake when CMakeLists.txt changes, so the
# CONFIGURE_DEPENDS flag keeps it accurate during development.
file(
    GLOB_RECURSE APP_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/third_party/*.cpp"
)

# =============================================================================
# Main executable
# =============================================================================

add_executable(lxpaint ${APP_SOURCES})

# C++20 is required (std::format, designated initialisers, ranges…)
target_compile_features(lxpaint PRIVATE cxx_std_20)

target_include_directories(lxpaint PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(
    lxpaint
    PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf imgui stb
)

# =============================================================================
# Compiler warnings & hardening
# =============================================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        lxpaint
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wno-unused-parameter # noisy in event-handler code
            "$<$<CONFIG:Debug>:-g3>"
            "$<$<CONFIG:Debug>:-O0>"
            "$<$<CONFIG:Release>:-O2>"
            "$<$<CONFIG:RelWithDebInfo>:-O2;-g>"
    )
endif()

if(MSVC)
    target_compile_options(lxpaint PRIVATE /W4 /permissive-)
endif()

# =============================================================================
# Assets — copied next to the binary at build time, installed alongside it at
# install time.
# =============================================================================

set(ASSETS_SRC "${CMAKE_SOURCE_DIR}/assets")

# Copy assets to the build tree so the binary can find them at runtime using
# relative paths (../assets/…) as the current code expects.
add_custom_command(
    TARGET lxpaint
    POST_BUILD
    COMMAND
        "${CMAKE_COMMAND}" -E copy_directory "${ASSETS_SRC}"
        "$<TARGET_FILE_DIR:lxpaint>/../assets"
    COMMENT "Copying assets to build directory"
)

# =============================================================================
# Installation rules
# =============================================================================

include(GNUInstallDirs)

install(TARGETS lxpaint RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Install assets relative to the binary
install(
    DIRECTORY "${ASSETS_SRC}/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/lxpaint/assets"
)

# Optional: desktop entry and icon (uncomment and adjust when ready)
# install(FILES "packaging/lxpaint.desktop" DESTINATION
# "${CMAKE_INSTALL_DATADIR}/applications") install(FILES "packaging/lxpaint.png"
# DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps")

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "  ┌─────────────────────────────────────────┐")
message(STATUS "  │  LXPaint ${PROJECT_VERSION} — ${CMAKE_BUILD_TYPE}")
message(STATUS "  │")
message(
    STATUS
    "  │  Compiler   : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
)
message(STATUS "  │  Build dir  : ${CMAKE_BINARY_DIR}")
message(STATUS "  │  Install to : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  └─────────────────────────────────────────┘")
message(STATUS "")
