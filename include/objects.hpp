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

#include <limits>
#include <optional>
#include <variant>
#include "math.hpp"
#include "ray.hpp"
#include "surface.hpp"
#include "transform.hpp"

namespace rt
{

    struct any_thing;

    struct intersection
    {
        const any_thing *thing_;
        ray ray_;
        real_t dist;
    };

    struct sphere
    {
        vec3 center;
        real_t radius;
        surface surf;
        transform xform;    // Add transformation matrix
        bool use_transform; // Flag to enable/disable transforms

        // Original constructor (no transform)
        constexpr sphere(const vec3 &center, real_t radius, const surface &surf)
            : center{center}, radius{radius}, surf{surf}, xform{}, use_transform{false} {}

        // New constructor with transform
        constexpr sphere(const surface &surf, const transform &xform)
            : center{0, 0, 0}, radius{1.0}, surf{surf}, xform{xform}, use_transform{true} {}

        constexpr std::optional<intersection> intersect(const ray &ray_) const
        {
            if (use_transform)
            {
                return intersect_transformed(ray_);
            }
            else
            {
                return intersect_original(ray_);
            }
        }

    private:
        // Original intersection method
        constexpr std::optional<intersection> intersect_original(const ray &ray_) const
        {
            const vec3 eo = center - ray_.start;
            const real_t v = dot(eo, ray_.dir);

            if (v < 0)
            {
                return std::nullopt;
            }
            else
            {
                const real_t disc = radius * radius - (dot(eo, eo) - v * v);
                if (disc < 0)
                {
                    return std::nullopt;
                }
                else
                {
                    const real_t d = cmath::sqrt(disc);
                    const real_t dist = v - d;
                    if (dist < 0)
                    {
                        return std::nullopt;
                    }
                    else
                    {
                        return intersection{nullptr, ray_, dist};
                    }
                }
            }
        }

        // Transform-aware intersection method
        std::optional<intersection> intersect_transformed(const ray &world_ray) const
        {
            // Transform ray to object space (unit sphere at origin)
            const vec3 obj_origin = xform.inv_point(world_ray.start);
            const vec3 obj_dir_unnorm = xform.inv_vector(world_ray.dir);
            const real_t scale_factor = mag(obj_dir_unnorm);
            const vec3 obj_dir = scale(1.0 / scale_factor, obj_dir_unnorm);

            const ray obj_ray{obj_origin, obj_dir};

            // Intersect unit sphere at origin: x² + y² + z² = 1
            const vec3 oc = obj_ray.start;
            const real_t a = dot(obj_ray.dir, obj_ray.dir);
            const real_t b = 2.0 * dot(oc, obj_ray.dir);
            const real_t c = dot(oc, oc) - 1.0;
            const real_t discriminant = b * b - 4 * a * c;

            if (discriminant < 0)
                return std::nullopt;

            const real_t sqrt_d = cmath::sqrt(discriminant);
            const real_t t1 = (-b - sqrt_d) / (2 * a);
            const real_t t2 = (-b + sqrt_d) / (2 * a);

            constexpr real_t eps = 1e-6;
            real_t t = 0; // Initialize the variable
            if (t1 > eps)
            {
                t = t1;
            }
            else if (t2 > eps)
            {
                t = t2;
            }
            else
            {
                return std::nullopt;
            }

            // Scale t back to world space
            const real_t world_t = t / scale_factor;
            return intersection{nullptr, world_ray, world_t};
        }

    public:
        constexpr vec3 get_normal(const vec3 &pos) const
        {
            if (use_transform)
            {
                // Transform to object space, get normal, transform back
                const vec3 obj_pos = xform.inv_point(pos);
                const vec3 obj_normal = norm(obj_pos); // Normal for unit sphere at origin
                return xform.normal(obj_normal);
            }
            else
            {
                return norm(pos - center);
            }
        }

        constexpr const surface &get_surface() const
        {
            return surf;
        }
    };

    struct plane
    {
        vec3 norm;
        real_t offset;
        surface surf;
        transform xform;    // Add transformation matrix
        bool use_transform; // Flag to enable/disable transforms

        // Original constructor (no transform)
        constexpr plane(const vec3 &norm, real_t offset, const surface &surf)
            : norm{norm}, offset{offset}, surf{surf}, xform{}, use_transform{false} {}

        // New constructor with transform
        constexpr plane(const surface &surf, const transform &xform)
            : norm{0, 1, 0}, offset{0}, surf{surf}, xform{xform}, use_transform{true} {}

        constexpr std::optional<intersection> intersect(const ray &ray_) const
        {
            if (use_transform)
            {
                return intersect_transformed(ray_);
            }
            else
            {
                return intersect_original(ray_);
            }
        }

    private:
        // Original intersection method
        constexpr std::optional<intersection> intersect_original(const ray &ray_) const
        {
            const real_t denom = dot(norm, ray_.dir);
            if (denom > 0)
            {
                return std::nullopt;
            }
            else
            {
                const real_t dist = (dot(norm, ray_.start) + offset) / (-denom);
                return intersection{nullptr, ray_, dist};
            }
        }

        // Transform-aware intersection method
        std::optional<intersection> intersect_transformed(const ray &world_ray) const
        {
            // Transform ray to object space (XZ plane at y=0)
            const vec3 obj_origin = xform.inv_point(world_ray.start);
            const vec3 obj_dir_unnorm = xform.inv_vector(world_ray.dir);
            const real_t scale_factor = mag(obj_dir_unnorm);
            const vec3 obj_dir = scale(1.0 / scale_factor, obj_dir_unnorm);

            const ray obj_ray{obj_origin, obj_dir};

            // Intersect XZ plane at y=0
            constexpr real_t eps = 1e-9;
            if (std::abs(obj_ray.dir.y) < eps)
                return std::nullopt;

            const real_t t = -obj_ray.start.y / obj_ray.dir.y;
            if (t <= 1e-6)
                return std::nullopt;

            // Scale t back to world space
            const real_t world_t = t / scale_factor;
            return intersection{nullptr, world_ray, world_t};
        }

    public:
        constexpr vec3 get_normal(const vec3 &pos) const
        {
            if (use_transform)
            {
                // For XZ plane, normal is always (0,1,0) in object space
                const vec3 obj_normal{0, 1, 0};
                return xform.normal(obj_normal);
            }
            else
            {
                return norm;
            }
        }

        constexpr const surface &get_surface() const
        {
            return surf;
        }
    };

    struct any_thing
    {
        std::variant<sphere, plane> thing;

        constexpr any_thing(const sphere &s) : thing{s} {}
        constexpr any_thing(const plane &p) : thing{p} {}

        constexpr std::optional<intersection> intersect(const ray &ray_) const // std::visit forwards call to the correct type in variant
        {
            return std::visit([&ray_, this](const auto &t)
                              { 
            auto result = t.intersect(ray_);
            if (result) {
                result->thing_ = this;
            }
            return result; }, thing);
        }

        constexpr vec3 get_normal(const vec3 &pos) const
        {
            return std::visit([&pos](const auto &t)
                              { return t.get_normal(pos); }, thing);
        }

        constexpr const surface &get_surface() const
        {
            return std::visit([](const auto &t) -> const surface &
                              { return t.get_surface(); }, thing);
        }
    };

    // Convenience factory functions
    constexpr sphere make_sphere(const vec3 &center, real_t radius, const surface &surf)
    {
        return sphere{center, radius, surf};
    }

    constexpr sphere make_transformed_sphere(const surface &surf, const transform &xform)
    {
        return sphere{surf, xform};
    }

    constexpr plane make_plane(const vec3 &normal, real_t offset, const surface &surf)
    {
        return plane{normal, offset, surf};
    }

    constexpr plane make_transformed_plane(const surface &surf, const transform &xform)
    {
        return plane{surf, xform};
    }

} // end namespace rt