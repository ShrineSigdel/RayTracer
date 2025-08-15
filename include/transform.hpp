/*
 * Transform system for raytracer
 * 4x4 transformation matrices with precomputed inverses
 */

#pragma once

#include "math.hpp"
#include "ray.hpp"
#include <cmath>

namespace rt
{

    // 4x4 transformation matrix
    struct transform
    {
        real_t m[16];    // Forward transform
        real_t inv[16];  // Inverse transform
        real_t inv_t[9]; // Inverse transpose 3x3 for normals

        // Identity constructor
        constexpr transform() : m{}, inv{}, inv_t{}
        {
            // Initialize as identity
            for (int i = 0; i < 16; ++i)
                m[i] = inv[i] = 0;
            for (int i = 0; i < 9; ++i)
                inv_t[i] = 0;

            m[0] = m[5] = m[10] = m[15] = 1; // Identity matrix
            inv[0] = inv[5] = inv[10] = inv[15] = 1;
            inv_t[0] = inv_t[4] = inv_t[8] = 1; // 3x3 identity
        }

        // Transform point (homogeneous w=1)
        constexpr vec3 point(const vec3 &p) const
        {
            return vec3{
                m[0] * p.x + m[1] * p.y + m[2] * p.z + m[3],
                m[4] * p.x + m[5] * p.y + m[6] * p.z + m[7],
                m[8] * p.x + m[9] * p.y + m[10] * p.z + m[11]};
        }

        // Transform vector (homogeneous w=0)
        constexpr vec3 vector(const vec3 &v) const
        {
            return vec3{
                m[0] * v.x + m[1] * v.y + m[2] * v.z,
                m[4] * v.x + m[5] * v.y + m[6] * v.z,
                m[8] * v.x + m[9] * v.y + m[10] * v.z};
        }

        // Inverse transform point
        constexpr vec3 inv_point(const vec3 &p) const
        {
            return vec3{
                inv[0] * p.x + inv[1] * p.y + inv[2] * p.z + inv[3],
                inv[4] * p.x + inv[5] * p.y + inv[6] * p.z + inv[7],
                inv[8] * p.x + inv[9] * p.y + inv[10] * p.z + inv[11]};
        }

        // Inverse transform vector
        constexpr vec3 inv_vector(const vec3 &v) const
        {
            return vec3{
                inv[0] * v.x + inv[1] * v.y + inv[2] * v.z,
                inv[4] * v.x + inv[5] * v.y + inv[6] * v.z,
                inv[8] * v.x + inv[9] * v.y + inv[10] * v.z};
        }

        // Transform normal (uses inverse transpose)
        constexpr vec3 normal(const vec3 &n) const
        {
            vec3 result{
                inv_t[0] * n.x + inv_t[1] * n.y + inv_t[2] * n.z,
                inv_t[3] * n.x + inv_t[4] * n.y + inv_t[5] * n.z,
                inv_t[6] * n.x + inv_t[7] * n.y + inv_t[8] * n.z};
            return norm(result);
        }

        // Factory methods
        static constexpr transform translate(real_t x, real_t y, real_t z)
        {
            transform t;
            t.m[3] = x;
            t.m[7] = y;
            t.m[11] = z;
            t.inv[3] = -x;
            t.inv[7] = -y;
            t.inv[11] = -z;
            return t;
        }

        static constexpr transform scale(real_t sx, real_t sy, real_t sz)
        {
            transform t;
            t.m[0] = sx;
            t.m[5] = sy;
            t.m[10] = sz;
            t.inv[0] = 1 / sx;
            t.inv[5] = 1 / sy;
            t.inv[10] = 1 / sz;
            // Inverse transpose for normals
            t.inv_t[0] = 1 / sx;
            t.inv_t[4] = 1 / sy;
            t.inv_t[8] = 1 / sz;
            return t;
        }

        static transform rotate_y(real_t radians)
        {
            transform t;
            const real_t c = std::cos(radians);
            const real_t s = std::sin(radians);

            t.m[0] = c;
            t.m[2] = s;
            t.m[8] = -s;
            t.m[10] = c;
            t.inv[0] = c;
            t.inv[2] = -s;
            t.inv[8] = s;
            t.inv[10] = c;
            t.inv_t[0] = c;
            t.inv_t[2] = -s;
            t.inv_t[6] = s;
            t.inv_t[8] = c;
            return t;
        }
    };

    // Compose transforms: result = second * first
    inline transform compose(const transform &first, const transform &second)
    {
        transform result;

        // Multiply 4x4 matrices: result.m = second.m * first.m
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                result.m[row * 4 + col] = 0;
                for (int k = 0; k < 4; ++k)
                {
                    result.m[row * 4 + col] += second.m[row * 4 + k] * first.m[k * 4 + col];
                }
            }
        }

        // Inverse: result.inv = first.inv * second.inv
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                result.inv[row * 4 + col] = 0;
                for (int k = 0; k < 4; ++k)
                {
                    result.inv[row * 4 + col] += first.inv[row * 4 + k] * second.inv[k * 4 + col];
                }
            }
        }

        // Inverse transpose 3x3 for normals
        for (int row = 0; row < 3; ++row)
        {
            for (int col = 0; col < 3; ++col)
            {
                result.inv_t[row * 3 + col] = 0;
                for (int k = 0; k < 3; ++k)
                {
                    result.inv_t[row * 3 + col] += first.inv[k * 4 + row] * second.inv[col * 4 + k];
                }
            }
        }

        return result;
    }

} // namespace rt
