cmake_minimum_required(VERSION 3.5)

project(raytracer C CXX)

# Find SDL2
find_package(SDL2 REQUIRED)

# Add include directory
include_directories(include)

# SDL renderer
add_executable(raytracer-sdl-advanced src/sdl_renderer_advanced.cpp)
target_link_libraries(raytracer-sdl-advanced SDL2::SDL2)

# Require C++17 - only for the target that exists
set_target_properties(raytracer-sdl-advanced PROPERTIES
                      CXX_STANDARD 17 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)