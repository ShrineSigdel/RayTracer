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


struct light {
    vec3 pos;
    color col;
};

//currently just a point light 

} // end namespace rt 