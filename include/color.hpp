
/*
 * Color utilities for ray tracer
 * Based on raytracer.ts from Microsoft TypeScript examples
 */

/*
 * TypeScript version copyright 2015-2017 Microsoft
 * C++ translation copyright 2017-2018 Tristan Brindle
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "math.hpp"

namespace rt {

struct color {
    real_t r;
    real_t g;
    real_t b;

    static constexpr color white() { return { 1.0, 1.0, 1.0 }; }
    static constexpr color grey() { return { 0.5, 0.5, 0.5 }; }
    static constexpr color black() { return {}; };
    static constexpr color background() { return black(); }
    static constexpr color default_color() { return black(); }
};

constexpr color scale(real_t k, const color& v) //adjust intensity of color(like bright dim )
{
    return { k * v.r, k * v.g, k * v.b };
}

constexpr color operator+(const color& v1, const color& v2) //used when multiple light sources contribute to a pixel
{
    return { v1.r + v2.r, v1.g + v2.g, v1.b + v2.b };
}

constexpr color operator*(const color& v1, const color& v2) //Applying surface color to light color.
{
    return {v1.r * v2.r, v1.g * v2.g, v1.b * v2.b};
}

} // end namespace rt 