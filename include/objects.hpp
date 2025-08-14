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

namespace rt {

struct any_thing;

struct intersection {
    const any_thing* thing_;
    ray ray_;
    real_t dist;
};

struct sphere {
    vec3 center;
    real_t radius;
    surface surf;

    constexpr sphere(const vec3& center, real_t radius, const surface& surf)
            : center{center}, radius{radius}, surf{surf} {}

    constexpr std::optional<intersection> intersect(const ray& ray_) const
    {
        const vec3 eo = center - ray_.start;
        const real_t v = dot(eo, ray_.dir);

        if (v < 0) {
            return std::nullopt;
        } else {
            const real_t disc = radius * radius - (dot(eo, eo) - v * v);
            if (disc < 0) {
                return std::nullopt;
            } else {
                const real_t d = cmath::sqrt(disc);
                const real_t dist = v - d;
                if (dist < 0) {
                    return std::nullopt;
                } else {
                    return intersection{nullptr, ray_, dist};
                }
            }
        }
    }

    constexpr vec3 get_normal(const vec3& pos) const
    {
        return norm(pos - center);
    }

    constexpr const surface& get_surface() const
    {
        return surf;
    }
};

struct plane {
    vec3 norm;
    real_t offset; //how far the plane is from the origin along its normal
    surface surf;

    constexpr plane(const vec3& norm, real_t offset, const surface& surf)
            : norm{norm}, offset{offset}, surf{surf} {}

    constexpr std::optional<intersection> intersect(const ray& ray_) const
    {
        const real_t denom = dot(norm, ray_.dir);
        if (denom > 0) {
            return std::nullopt;
        } else {
            const real_t dist = (dot(norm, ray_.start) + offset) / (-denom);
            return intersection{nullptr, ray_, dist};
        }
    }

    constexpr vec3 get_normal(const vec3&) const
    {
        return norm;
    }

    constexpr const surface& get_surface() const
    {
        return surf;
    }
};

struct any_thing {
    std::variant<sphere, plane> thing;

    constexpr any_thing(const sphere& s) : thing{s} {}
    constexpr any_thing(const plane& p) : thing{p} {}

    constexpr std::optional<intersection> intersect(const ray& ray_) const //std::visit forwards call to the correct type in variant
    {
        return std::visit([&ray_, this](const auto& t) { 
            auto result = t.intersect(ray_);
            if (result) {
                result->thing_ = this;
            }
            return result;
        }, thing);
    }

    constexpr vec3 get_normal(const vec3& pos) const
    {
        return std::visit([&pos](const auto& t) { return t.get_normal(pos); }, thing);
    }

    constexpr const surface& get_surface() const
    {
        return std::visit([](const auto& t) -> const surface& { return t.get_surface(); }, thing);
    }
};

} // end namespace rt 