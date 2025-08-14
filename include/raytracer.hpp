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

// Include all modular components
#include "math.hpp"
#include "color.hpp"
#include "ray.hpp"
#include "light.hpp"
#include "surface.hpp"
#include "objects.hpp"

namespace rt {

class ray_tracer {
private:
    static constexpr int max_depth = 5; //no of times to trace a ray after reflection

    template <typename Scene> //scene is not declard here, it is a template and can be anything but we will create a scene later with approproate methods that are used here
    constexpr std::optional<intersection> get_intersections(const ray& ray_, const Scene& scene_) const
    {
        std::optional<intersection> closest_inter = std::nullopt;
        real_t closest_dist = std::numeric_limits<real_t>::max();

        for (const auto& thing : scene_.get_things()) {
            if (const auto inter = thing.intersect(ray_); inter) { //the intersect returns a pointer of intersection
                if ((*inter).dist < closest_dist) {
                    closest_dist = (*inter).dist;
                    closest_inter = std::move(inter); 
                }
            }
        }

        return closest_inter; //return the closest intersection
    }

    template <typename Scene>
    constexpr std::optional<real_t> test_ray(const ray& ray_, const Scene& scene_) const
    {
        if (const auto isect = get_intersections(ray_, scene_); isect) {
            return isect->dist;
        }
        return std::nullopt;
    }

    template <typename Scene>
    constexpr color trace_ray(const ray& ray_, const Scene& scene_, int depth) const
    {
        if (const auto isect = get_intersections(ray_, scene_); isect) {
            return shade(*isect, scene_, depth);
        }
        return color::background();
    }

    template <typename Scene>
    constexpr color shade(const intersection& isect, const Scene& scene, int depth) const
    {
        const vec3& d = isect.ray_.dir; //intersecting rays direction
        const vec3 pos = (isect.dist * d) + isect.ray_.start; //intersection pos
        const vec3 normal = isect.thing_->get_normal(pos);//normal at intersection point of object
        const vec3 reflect_dir = d - (2 * (dot(normal, d) * normal));
        const color natural_color = color::background() + get_natural_color(*isect.thing_, pos, normal, reflect_dir, scene);
        const color reflected_color = depth >= max_depth ? color::grey() : get_reflection_color(*isect.thing_, pos, reflect_dir, scene, depth);
        return natural_color + reflected_color;
    }

    template <typename Scene>
    constexpr color get_reflection_color(const any_thing& thing_, const vec3& pos,
                                         const vec3& rd, const Scene& scene, int depth) const
    {
        return scale(thing_.get_surface().reflect(pos), trace_ray({pos, rd }, scene, depth + 1));
    }

    template <typename Scene>
    constexpr color add_light(const any_thing& thing, const vec3& pos, const vec3& normal,
                              const vec3& rd, const Scene& scene, const color& col,
                              const light& light_) const
    {
        const vec3 ldis = light_.pos - pos;
        const vec3 livec = norm(ldis);
        const auto near_isect = test_ray({pos, livec}, scene);
        const bool is_in_shadow = near_isect ? *near_isect < mag(ldis) : false;
        if (is_in_shadow) {
            return col;
        }
        const auto illum = dot(livec, normal);
        const auto lcolor = (illum > 0) ? scale(illum, light_.col) : color::default_color();
        const auto specular = dot(livec, norm(rd));
        const auto& surf = thing.get_surface();
        const auto scolor = (specular > 0) ? scale(cmath::pow(specular, surf.roughness), light_.col)
                                           : color::default_color();
        return col + (surf.diffuse(pos) * lcolor) + (surf.specular(pos) * scolor);
    }

    template <typename Scene>
    constexpr color get_natural_color(const any_thing& thing, const vec3& pos,
                                      const vec3& norm_, const vec3& rd, const Scene& scene) const
    {
        color col = color::default_color();
        for (const auto& light : scene.get_lights()) {
            col = add_light(thing, pos, norm_, rd, scene, col, light);
        }
        return col;
    }

    constexpr vec3 get_point(int width, int height, int x, int y, const camera& cam) const
    {
        const auto recenterX =  (x - (width / 2.0)) / 2.0 / width;
        const auto recenterY = -(y - (height / 2.0)) / 2.0 / height;
        return norm(cam.forward + ((recenterX * cam.right) + (recenterY * cam.up)));
    }

public:
    template <typename Scene, typename Canvas>
    constexpr void render(const Scene& scene, Canvas& canvas, int width, int height) const
    {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const auto point = get_point(width, height, x, y, scene.get_camera());
                const auto color = trace_ray({ scene.get_camera().pos, point }, scene, 0);
                canvas.set_pixel(x, y, color);
            }
        }
    }
};

} // end namespace rt
