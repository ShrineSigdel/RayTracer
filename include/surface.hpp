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
#include "color.hpp"

namespace rt {

struct surface {
    using diffuse_func_t = color (*)(const vec3&); //typedef for function pointers
    using specular_func_t = color (*)(const vec3&);
    using reflect_func_t = real_t (*)(const vec3&);

    diffuse_func_t diffuse = nullptr;
    specular_func_t specular = nullptr;
    reflect_func_t reflect = nullptr;
    int roughness = 0;
};

namespace surfaces {

constexpr color shiny_diffuse(const vec3&)
{
    return color::white();
}

constexpr color shiny_specular(const vec3&)
{
    return color::grey();
}

constexpr real_t shiny_reflect(const vec3&)
{
    return 0.7;
}

constexpr color checkerboard_diffuse(const vec3& pos)
{
    if ((static_cast<int>(cmath::floor(pos.z)) + static_cast<int>(cmath::floor(pos.x))) % 2 != 0) {
        return color::white();
    } else {
        return color::black();
    }
}

constexpr color checkerboard_specular(const vec3&)
{
    return color::white();
}

constexpr real_t checkerboard_reflect(const vec3& pos)
{
    if ((static_cast<int>(cmath::floor(pos.z)) + static_cast<int>(cmath::floor(pos.x))) % 2 != 0) {
        return 0.1;
    } else {
        return 0.7;
    }
}

constexpr surface shiny = {
    shiny_diffuse,
    shiny_specular,
    shiny_reflect,
    100
};

constexpr surface checkerboard = {
    checkerboard_diffuse,
    checkerboard_specular,
    checkerboard_reflect,
    1
};

} // end namespace surfaces

} // end namespace rt 