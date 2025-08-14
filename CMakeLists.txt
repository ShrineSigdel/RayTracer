
cmake_minimum_required(VERSION 3.5)

project(raytracer C CXX)

# Find SDL2
find_package(SDL2 REQUIRED)

# Add include directory
include_directories(include)

# Compile-time renderer (PNG output)
add_executable(raytracer-ct src/compile_time.cpp)
target_compile_definitions(raytracer-ct PRIVATE "IMAGE_WIDTH=32;IMAGE_HEIGHT=32")
if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    target_compile_options(raytracer-ct PRIVATE -Wall -Wextra -fconstexpr-steps=2147483647)
elseif(${CMAKE_COMPILER_IS_GNUCXX})
    target_compile_options(raytracer-ct PRIVATE -Wall -Wextra -fconstexpr-ops-limit=2147483647)
endif()

# Runtime renderer (PNG output)
add_executable(raytracer-rt src/run_time.cpp)

# SDL renderer
add_executable(raytracer-sdl-advanced src/sdl_renderer_advanced.cpp)
target_link_libraries(raytracer-sdl-advanced SDL2::SDL2)

# Require C++17
set_target_properties(raytracer-ct raytracer-rt raytracer-sdl-advanced PROPERTIES
                      CXX_STANDARD 17 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)
