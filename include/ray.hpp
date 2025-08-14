/*
 * Modified by Roshan Rijal, 2025
 * 
 * This file is part of a fork of the "raytracer.hpp" project by Tristan Brindle,
 * which itself is a C++ translation of Microsoft's TypeScript Ray Tracer example.
 *
 * Original TypeScript version:
 * https://github.com/microsoft/TypeScriptSamples/tree/main/raytracer
 *
 * Licensed under the Apache License, Version 2.0
 * http://www.apache.org/licenses/LICENSE-2.0
 */


#pragma once

#include "math.hpp"

namespace rt {

struct ray {
    vec3 start;
    vec3 dir;
};

struct camera {
    vec3 pos;
    vec3 forward;
    vec3 right;
    vec3 up;

    constexpr camera(const vec3& pos, const vec3& look_at)
            : pos{pos},
              forward{norm(look_at - pos)},
              right{1.5 * norm(cross(forward, {0.0, -1.0, 0.0}))},
              up{1.5 * norm(cross(forward, right))}
    {}
};

} // end namespace rt 