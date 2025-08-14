# C++17 Ray Tracer with SDL2 Display

This is a modern C++ ray tracer that can render images in real-time using SDL2 windows. It is based on the original [raytracer.hpp](https://github.com/tcbrindle/raytracer.hpp) project by Tristan Brindle, with additional features and improvements.

## Features

- **Real-time SDL2 Display**: Render scenes interactively in a resizable SDL2 window.
- **Customizable Scenes**: Easily modify or create new scenes with objects, lights, and camera settings.
- **Multi-threading**: Advanced renderer with background threading for real-time updates.
- **Modern C++**: Uses C++17 features like `constexpr`, `std::variant`, and `std::optional`.

---

## Project Structure

```
raytracer.hpp/
├── include/           # Header files
│   ├── raytracer.hpp  # Main ray tracer implementation
│   ├── math.hpp       # Core math utilities (vectors, operations)
│   ├── color.hpp      # Color representation and manipulation
│   ├── ray.hpp        # Ray representation
│   ├── light.hpp      # Light sources
│   ├── surface.hpp    # Surface properties (diffuse, specular, reflectivity)
│   ├── objects.hpp    # Scene objects (spheres, planes)
├── src/               # Source files
│   ├── sdl_renderer_advanced.cpp # Real-time SDL renderer
├── CMakeLists.txt     # Build configuration
├── LICENSE.txt        # License information
└── README.md          # This file
```

---

## Prerequisites

### Dependencies
- **C++17 compiler** (e.g., GCC 7+, Clang 5+)
- **CMake** (version 3.5 or higher)
- **SDL2** (development libraries)

On Ubuntu, you can install the dependencies with:
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev
```

---

## Building the Project

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd raytracer.hpp
   ```

2. Create a build directory and configure the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. Build the project:
   ```bash
   make
   ```

---

## Running the Project


### Real-time SDL Renderer
```bash
./raytracer-sdl-advanced 1024 768
```
Opens a window with the rendered scene. You can:
- Press `ESC` to exit.
- Resize the window to see the scene scale dynamically.

---

## Modifications in This Fork

This fork introduces the following features:
1. **Real-time SDL Renderer**:
   - Multi-threaded rendering for real-time updates.
   - Interactive controls and resizable window support.
2. **Customizable Scenes**:
   - Easily add or modify objects, lights, and camera settings.
3. **Improved Documentation**:
   - Clear instructions for building, running, and modifying the project.

---

## License

This project is licensed under the Apache License 2.0. See the `LICENSE.txt` file for details.

---

## Credits

This project is based on the original [raytracer.hpp](https://github.com/tcbrindle/raytracer.hpp) by Tristan Brindle,  
which was translated from Microsoft's [TypeScript Ray Tracer example](https://github.com/microsoft/TypeScriptSamples/tree/main/raytracer).
