# Ray Tracer - Architecture and Transformation System

A C++17 header-only ray tracer with SDL2 real-time rendering and comprehensive 4x4 transformation matrices.

## Table of Contents

- [Overview](#overview)
- [Original Architecture (Bottom-Up)](#original-architecture-bottom-up)
- [New Transformation System](#new-transformation-system)
- [How It Works Now (Top-Down)](#how-it-works-now-top-down)
- [Usage Examples](#usage-examples)
- [Building and Running](#building-and-running)

## Overview

This ray tracer implements a physically-based rendering pipeline with support for:

- Sphere and plane primitives
- Phong lighting model with multiple light sources
- Reflections and shadows
- Surface materials (shiny, checkerboard, etc.)
- **NEW**: 4x4 transformation matrices for object positioning, scaling, and rotation
- Real-time SDL2 rendering with interactive window

## Original Architecture (Bottom-Up)

### Foundation Layer: Mathematics (`math.hpp`)

The mathematical foundation provides:

```cpp
// Core types
using real_t = double;
struct vec3 { real_t x, y, z; };

// Vector operations (all constexpr)
constexpr vec3 operator+(const vec3& a, const vec3& b);
constexpr vec3 operator*(real_t k, const vec3& v);
constexpr real_t dot(const vec3& a, const vec3& b);
constexpr vec3 norm(const vec3& v);  // Normalize
constexpr real_t mag(const vec3& v); // Magnitude
```

**How it worked before**: All vector math was performed in world space. Objects were defined with their final world positions directly.

### Rendering Primitives

#### Ray Representation (`ray.hpp`)

```cpp
struct ray {
    vec3 start;  // Ray origin
    vec3 dir;    // Ray direction (normalized)
};
```

#### Color System (`color.hpp`)

```cpp
struct color {
    real_t r, g, b;
    static constexpr color background() { return {0.0, 0.0, 0.0}; }
    static constexpr color white() { return {1.0, 1.0, 1.0}; }
};
```

#### Lighting (`light.hpp`)

```cpp
struct light {
    vec3 pos;    // Light position in world space
    color col;   // Light color/intensity
};
```

### Surface Materials (`surface.hpp`)

Defines how objects respond to light:

```cpp
struct surface {
    std::function<color(vec3)> diffuse;   // Diffuse color at point
    std::function<color(vec3)> specular;  // Specular color at point
    std::function<real_t(vec3)> reflect;  // Reflection coefficient
    real_t roughness;                     // Specular exponent
};

// Pre-defined materials
namespace surfaces {
    extern const surface shiny;
    extern const surface checkerboard;
}
```

### Geometric Objects (`objects.hpp` - Original)

**Before transformation system**:

```cpp
// Simple sphere in world coordinates
struct sphere {
    vec3 center;    // World space center
    real_t radius;  // Radius
    surface surf;   // Material

    constexpr std::optional<intersection> intersect(const ray& ray_) const {
        // Ray-sphere intersection in world space
        const vec3 eo = center - ray_.start;
        // ... intersection math directly in world coordinates
    }
};

// Infinite plane in world coordinates
struct plane {
    vec3 norm;      // World space normal
    real_t offset;  // Distance from origin
    surface surf;   // Material

    constexpr std::optional<intersection> intersect(const ray& ray_) const {
        // Ray-plane intersection in world space
        // ... intersection math directly in world coordinates
    }
};
```

**Limitation**: Objects could only exist at their defined world positions. No easy way to:

- Move objects after creation
- Scale objects
- Rotate objects
- Create instances of the same object at different positions

### Ray Tracing Engine (`raytracer.hpp`)

The core rendering algorithm:

1. **Ray Generation**: For each pixel, generate a ray from camera through the pixel
2. **Intersection Testing**: Find closest intersection with all objects
3. **Shading**: Calculate color using Phong lighting model
4. **Reflection**: Recursively trace reflected rays (up to max depth)
5. **Shadow Testing**: Cast shadow rays to lights to determine occlusion

```cpp
class ray_tracer {
    template <typename Scene>
    constexpr color trace_ray(const ray& ray_, const Scene& scene_, int depth) const {
        if (const auto isect = get_intersections(ray_, scene_); isect) {
            return shade(*isect, scene_, depth);  // Phong shading + reflections
        }
        return color::background();
    }
};
```

### Scene and Rendering (`sdl_renderer_advanced.cpp`)

**Original scene setup**:

```cpp
struct sdl_scene {
    std::vector<any_thing> things_{
        plane{{ 0.0, 1.0, 0.0 }, 0.0, surfaces::checkerboard},  // Floor
        sphere{{ 0.0, 1.0, -0.25 }, 1.0, surfaces::shiny},      // Center sphere
        sphere{{ -1.0, 0.5, 1.5 }, 0.5, surfaces::shiny}       // Side sphere
    };
    // ... lights and camera
};
```

**Problems with original approach**:

- Hard-coded object positions
- No way to easily duplicate or transform objects
- Scene composition was inflexible
- Difficult to animate or modify object properties

---

## New Transformation System

### Transform Mathematics (`transform.hpp`)

**New addition**: Complete 4x4 transformation matrix system:

```cpp
struct transform {
    real_t m[16];     // Forward transform matrix (column-major)
    real_t inv[16];   // Pre-computed inverse matrix
    real_t inv_t[9];  // Inverse transpose for normal transformation

    // Transform operations
    constexpr vec3 point(const vec3& p) const;     // Transform point (w=1)
    constexpr vec3 vector(const vec3& v) const;    // Transform vector (w=0)
    constexpr vec3 normal(const vec3& n) const;    // Transform normal (special handling)

    // Inverse transforms for ray transformation
    constexpr vec3 inv_point(const vec3& p) const;
    constexpr vec3 inv_vector(const vec3& v) const;
};
```

**Factory methods for common transformations**:

```cpp
// Translation
static constexpr transform translate(real_t x, real_t y, real_t z);

// Uniform and non-uniform scaling
static constexpr transform scale(real_t sx, real_t sy, real_t sz);

// Y-axis rotation (extensible to other axes)
static transform rotate_y(real_t radians);

// Composition (combines multiple transforms)
transform compose(const transform& first, const transform& second);
```

**Key mathematical insights**:

- **Points vs Vectors**: Points have w=1 (affected by translation), vectors have w=0 (not affected)
- **Normal Transformation**: Normals require inverse transpose to remain perpendicular to surfaces
- **Ray Transformation**: Rays are transformed to object space for intersection testing
- **Pre-computed Inverses**: Avoids expensive matrix inversion during rendering

### Enhanced Objects (`objects.hpp` - New)

**After transformation system**:

```cpp
struct sphere {
    vec3 center;     // Object space center (usually origin)
    real_t radius;   // Object space radius
    surface surf;    // Material
    transform xform; // NEW: World transformation

    // Backward compatibility constructor
    constexpr sphere(vec3 center_, real_t radius_, surface surf_)
        : center(center_), radius(radius_), surf(surf_), xform() {}

    // New transformation-aware constructor
    constexpr sphere(vec3 center_, real_t radius_, surface surf_, transform xform_)
        : center(center_), radius(radius_), surf(surf_), xform(xform_) {}

    constexpr std::optional<intersection> intersect(const ray& ray_) const {
        // Transform ray to object space
        const ray obj_ray{
            xform.inv_point(ray_.start),   // Transform start point
            xform.inv_vector(ray_.dir)     // Transform direction
        };

        // Perform intersection in object space (simpler math)
        // ... standard ray-sphere intersection

        // Transform intersection back to world space
        if (intersection found) {
            isect.thing_ = this;
            isect.ray_ = ray_;  // Original world space ray
            return isect;
        }
    }

    constexpr vec3 get_normal(const vec3& pos) const {
        // Transform world position to object space
        const vec3 obj_pos = xform.inv_point(pos);
        // Calculate object space normal
        const vec3 obj_normal = norm(obj_pos - center);
        // Transform normal back to world space
        return xform.normal(obj_normal);
    }
};
```

**Benefits of new approach**:

- **Object Space Math**: Intersection calculations use simple, canonical object definitions
- **Flexible Positioning**: Objects can be easily moved, scaled, and rotated
- **Backward Compatibility**: Existing code continues to work unchanged
- **Reusability**: Same object definition can be instanced multiple times with different transforms

---

## How It Works Now (Top-Down)

### 1. Application Layer (`main()`)

```cpp
int main() {
    // Create SDL window and renderer
    AdvancedSDLRenderer sdl_renderer(800, 600);

    // Create scene and canvas
    sdl_scene scene{};
    sdl_canvas canvas(800, 600);

    // Render in separate thread
    ray_tracer tracer{};
    tracer.render(scene, canvas, 800, 600);

    // Interactive event loop with real-time display
}
```

### 2. Scene Definition

```cpp
struct sdl_scene {
    std::vector<any_thing> things_{
        // Floor plane (no transform needed)
        plane{{ 0.0, 1.0, 0.0 }, 0.0, surfaces::checkerboard},

        // Objects can now use transforms for positioning
        sphere{{ 0.0, 0.0, 0.0 }, 1.0, surfaces::shiny,
                transform::translate(0.0, 1.0, -0.25)},

        sphere{{ 0.0, 0.0, 0.0 }, 0.5, surfaces::shiny,
                compose(transform::scale(1.0, 1.0, 1.0),
                       transform::translate(-1.0, 0.5, 1.5))}
    };
    // ... lights and camera remain the same
};
```

### 3. Ray Tracing Pipeline

#### For Each Pixel:

1. **Camera Ray Generation**: Generate ray from camera through pixel

   ```cpp
   ray camera_ray = { camera.pos, pixel_direction };
   ```

2. **Intersection Testing**: For each object in scene

   ```cpp
   // NEW: Transform ray to object space
   ray obj_ray = { transform.inv_point(ray.start),
                   transform.inv_vector(ray.dir) };

   // Perform intersection in simple object space
   intersection = sphere.intersect_object_space(obj_ray);

   // Results automatically in world space coordinates
   ```

3. **Closest Intersection**: Find nearest intersection along ray

4. **Shading Calculation**: At intersection point

   ```cpp
   // World space position
   vec3 world_pos = intersection.dist * ray.dir + ray.start;

   // World space normal (transformed from object space)
   vec3 world_normal = object.get_normal(world_pos);

   // Phong lighting in world space
   color final_color = phong_lighting(world_pos, world_normal, lights);
   ```

5. **Reflection Tracing**: If surface is reflective
   ```cpp
   ray reflect_ray = { world_pos, reflection_direction };
   color reflect_color = trace_ray(reflect_ray, depth + 1);
   ```

### 4. Mathematical Transform Pipeline

For a sphere at object-space origin with world transform:

```
Object Definition (Object Space):
center = (0, 0, 0)
radius = 1.0

World Transform:
T = translate(2, 1, 0) * scale(1.5, 1.0, 1.0) * rotate_y(45°)

Intersection Process:
1. World ray: start=(0,0,5), dir=(0,0,-1)
2. Transform to object space: T⁻¹ * ray
3. Object space ray: start=(-1.33,0,3.33), dir=(0.707,0,-0.707)
4. Intersection math in simple object space
5. Transform result back to world space
```

### 5. Surface Material Evaluation

Materials now work seamlessly with transformed objects:

```cpp
// Checkerboard pattern adapts to object scaling/rotation
color checkerboard_color = surfaces::checkerboard.diffuse(world_pos);

// Specular highlights remain correct due to proper normal transformation
real_t specular_intensity = surfaces::shiny.specular(world_pos);
```

---

## Usage Examples

### Basic Object Creation

```cpp
// Simple sphere (backward compatible)
sphere simple{vec3{0, 1, 0}, 1.0, surfaces::shiny};

// Transformed sphere
sphere translated{
    vec3{0, 0, 0}, 1.0, surfaces::shiny,
    transform::translate(2.0, 1.0, -3.0)
};
```

### Complex Transformations

```cpp
// Scaled and rotated sphere
transform complex_xform = compose(
    transform::rotate_y(45.0 * M_PI / 180.0),  // 45 degree rotation
    compose(
        transform::scale(2.0, 1.0, 1.0),       // Ellipsoid (2x scale in X)
        transform::translate(0.0, 2.0, 0.0)    // Move up 2 units
    )
);

sphere ellipsoid{vec3{0,0,0}, 1.0, surfaces::shiny, complex_xform};
```

### Scene Composition

```cpp
// Create multiple instances of same object
auto red_material = /* custom surface */;
auto base_sphere = sphere{vec3{0,0,0}, 0.5, red_material};

std::vector<any_thing> spheres;
for (int i = 0; i < 5; ++i) {
    spheres.emplace_back(sphere{
        vec3{0,0,0}, 0.5, red_material,
        transform::translate(i * 2.0, 0.5, 0.0)
    });
}
```

---

## Building and Running

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
