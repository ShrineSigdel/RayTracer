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

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rt
{

    using real_t = float; // similar to typedef float real_t (using keyword can be used for aliases too)

    // Constexpr maths functions.
    namespace cmath
    {

// libstdc++ provides some constexpr math functions as an extension, so
// use them if we can.
#if defined(__GLIBCXX__) && !defined(__clang__)
#define HAVE_CONSTEXPR_STD_MATH
#endif

        // Compile-time square root using Newton-Raphson
        constexpr real_t sqrt(real_t val)
        {
#ifdef HAVE_CONSTEXPR_STD_MATH
            return std::sqrt(val);
#else
            real_t curr = val;
            real_t prev = 0;

            while (curr != prev)
            {
                prev = curr;
                curr = 0.5 * (curr + val / curr);
            }

            return curr;
#endif
        }

        constexpr real_t floor(real_t val)
        {
#ifdef HAVE_CONSTEXPR_STD_MATH
            return std::floor(val);
#else
            // This is wrong for anything outside the range of intmax_t
            return static_cast<intmax_t>(val >= 0.0 ? val : val - 1.0);
#endif
        }

        constexpr real_t pow(real_t base, int iexp)
        {
            real_t val{1.0};

            while (iexp-- > 0)
            {
                val *= base;
            }

            return val;
        }

    } // end namespace cmath

    struct vec3
    {
        real_t x;
        real_t y;
        real_t z;
    };

    constexpr vec3 operator*(real_t k, const vec3 &v)
    {
        return {k * v.x, k * v.y, k * v.z};
    }

    constexpr vec3 scale(real_t k, const vec3 &v)
    {
        return {k * v.x, k * v.y, k * v.z};
    }

    constexpr vec3 operator-(const vec3 &v1, const vec3 &v2)
    {
        return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    }

    constexpr vec3 operator+(const vec3 &v1, const vec3 &v2)
    {
        return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    }

    constexpr real_t dot(const vec3 &v1, const vec3 &v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    constexpr real_t mag(const vec3 &v)
    {
        return cmath::sqrt(dot(v, v));
    }

    constexpr vec3 norm(const vec3 &v)
    {
        return (real_t{1.0} / mag(v)) * v;
    }

    constexpr vec3 cross(const vec3 &v1, const vec3 &v2)
    {
        return {v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x};
    }

} // end namespace rt