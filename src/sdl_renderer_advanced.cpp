#include "../include/raytracer.hpp"

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

using namespace rt;

namespace {

struct sdl_scene {
private:
    std::vector<any_thing> things_{
            plane{{ 0.0, 1.0, 0.0 }, 0.0, surfaces::checkerboard},
            sphere{{ 0.0, 1.0, -0.25 }, 1.0, surfaces::shiny},
            sphere{{ -1.0, 0.5, 1.5 }, 0.5, surfaces::shiny}
    };
    std::vector<light> lights_{
            light{{-2.0, 2.5, 0.0}, {0.49, 0.07, 0.07}},
            light{{1.5, 2.5, 1.5}, {0.07, 0.07, 0.49}},
            light{{1.5, 2.5, -1.5}, {0.07, 0.49, 0.071}},
            light{{0.0, 3.5, 0.0}, {0.21, 0.21, 0.35}}
    };
    camera cam_{vec3{ 3.0, 2.0, 4.0 }, vec3{ -1.0, 0.5, 0.0 }};

public:
    const auto& get_things() const { return things_; }
    const auto& get_lights() const { return lights_; }
    const auto& get_camera() const { return cam_; }
};

struct sdl_canvas {
    std::vector<uint32_t> pixels_;
    int width_;
    int height_;

    sdl_canvas(int width, int height) 
        : width_(width), height_(height), pixels_(width * height) {}

    void set_pixel(int x, int y, color col) {
        if (x >= 0 && x < width_ && y >= 0 && y < height_) {
            const auto clamp = [](real_t val) { 
                return std::clamp<real_t>(val, 0.0, 1.0); 
            };
            
            uint8_t r = static_cast<uint8_t>(clamp(col.r) * 255.0);
            uint8_t g = static_cast<uint8_t>(clamp(col.g) * 255.0);
            uint8_t b = static_cast<uint8_t>(clamp(col.b) * 255.0);
            
            // Convert to RGBA8888 format
            pixels_[x + width_ * y] = (r << 24) | (g << 16) | (b << 8) | 0xFF;
        }
    }

    const std::vector<uint32_t>& get_pixels() const { return pixels_; }
    int width() const { return width_; }
    int height() const { return height_; }
};

class AdvancedSDLRenderer {
private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    int render_width_;
    int render_height_;
    int window_width_;
    int window_height_;
    std::atomic<bool> should_stop_;

public:
    AdvancedSDLRenderer(int render_width, int render_height, int window_width = 0, int window_height = 0) 
        : render_width_(render_width), render_height_(render_height), should_stop_(false) {
        
        // If window size not specified, calculate based on render size with aspect ratio
        if (window_width == 0 || window_height == 0) {
            // Target a larger window size while maintaining aspect ratio
            const int max_window_size = 1200;
            const float aspect_ratio = static_cast<float>(render_width) / render_height;
            
            if (aspect_ratio > 1.0f) {
                // Landscape
                window_width_ = max_window_size;
                window_height_ = static_cast<int>(max_window_size / aspect_ratio);
            } else {
                // Portrait
                window_height_ = max_window_size;
                window_width_ = static_cast<int>(max_window_size * aspect_ratio);
            }
        } else {
            window_width_ = window_width;
            window_height_ = window_height;
        }

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("SDL could not initialize: " + std::string(SDL_GetError()));
        }

        window_ = SDL_CreateWindow(
            "Advanced Ray Tracer",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            window_width_, window_height_,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );

        if (!window_) {
            throw std::runtime_error("Window could not be created: " + std::string(SDL_GetError()));
        }

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer_) {
            throw std::runtime_error("Renderer could not be created: " + std::string(SDL_GetError()));
        }

        // Enable linear filtering for better scaling
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

        texture_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STATIC,
            render_width_, render_height_
        );

        if (!texture_) {
            throw std::runtime_error("Texture could not be created: " + std::string(SDL_GetError()));
        }
    }

    ~AdvancedSDLRenderer() {
        if (texture_) SDL_DestroyTexture(texture_);
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
        SDL_Quit();
    }

    void render(const std::vector<uint32_t>& pixels) {
        SDL_UpdateTexture(texture_, nullptr, pixels.data(), render_width_ * sizeof(uint32_t));
        SDL_RenderClear(renderer_);
        
        // Calculate destination rectangle to maintain aspect ratio
        SDL_Rect dest_rect = calculate_dest_rect();
        SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
        SDL_RenderPresent(renderer_);
    }

    void stop() {
        should_stop_ = true;
    }

    bool should_stop() const {
        return should_stop_.load();
    }

    bool handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        return false;
                    case SDLK_s:
                        // Save image
                        saveImage();
                        break;
                    case SDLK_SPACE:
                        // Toggle pause/resume
                        break;
                }
            }
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    window_width_ = event.window.data1;
                    window_height_ = event.window.data2;
                }
            }
        }
        return true;
    }

private:
    void saveImage() {
        // This would need to be implemented to save the current image
        std::cout << "Save functionality would be implemented here" << std::endl;
    }

    SDL_Rect calculate_dest_rect() {
        // Calculate the destination rectangle to maintain aspect ratio
        float window_aspect = static_cast<float>(window_width_) / window_height_;
        float render_aspect = static_cast<float>(render_width_) / render_height_;
        
        SDL_Rect dest_rect;
        
        if (window_aspect > render_aspect) {
            // Window is wider than render, fit to height
            dest_rect.h = window_height_;
            dest_rect.w = static_cast<int>(window_height_ * render_aspect);
            dest_rect.x = (window_width_ - dest_rect.w) / 2;
            dest_rect.y = 0;
        } else {
            // Window is taller than render, fit to width
            dest_rect.w = window_width_;
            dest_rect.h = static_cast<int>(window_width_ / render_aspect);
            dest_rect.x = 0;
            dest_rect.y = (window_height_ - dest_rect.h) / 2;
        }
        
        return dest_rect;
    }
};

} // namespace

int main(int argc, char* argv[]) {
    int render_width = 800;
    int render_height = 600;

    if (argc >= 3) {
        render_width = std::atoi(argv[1]);
        render_height = std::atoi(argv[2]);
    }

    try {
        std::cout << "Advanced SDL Ray Tracer" << std::endl;
        std::cout << "Rendering " << render_width << "x" << render_height << " image..." << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  ESC - Exit" << std::endl;
        std::cout << "  S - Save image" << std::endl;
        std::cout << "  Window is resizable - try resizing it!" << std::endl;
        std::cout << std::endl;
        
        // Create SDL renderer with larger window
        AdvancedSDLRenderer sdl_renderer(render_width, render_height);
        
        // Create canvas and scene
        sdl_canvas canvas(render_width, render_height);
        sdl_scene scene{};
        
        // Start rendering in a separate thread
        std::thread render_thread([&]() {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            ray_tracer tracer{};
            tracer.render(scene, canvas, render_width, render_height);
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << "Rendering completed in " << total_time.count() << "ms" << std::endl;
        });
        
        // Main event loop
        bool running = true;
        auto last_update = std::chrono::steady_clock::now();
        
        while (running && !sdl_renderer.should_stop()) {
            running = sdl_renderer.handleEvents();
            
            // Update display every 16ms (60 FPS)
            auto now = std::chrono::steady_clock::now();
            if (now - last_update > std::chrono::milliseconds(16)) {
                sdl_renderer.render(canvas.get_pixels());
                last_update = now;
            }
            
            SDL_Delay(1);
        }
        
        // Wait for rendering to complete
        if (render_thread.joinable()) {
            render_thread.join();
        }
        
        // Final render
        sdl_renderer.render(canvas.get_pixels());
        
        std::cout << "Rendering complete! Press ESC to exit." << std::endl;
        
        // Keep window open until user closes it
        while (running) {
            running = sdl_renderer.handleEvents();
            SDL_Delay(16);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 