#ifndef SDL_HPP
#define SDL_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_messagebox.h>

#include <string>
#include <sstream>
#include <iostream>
#include <memory>

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define SDL_QUIT_WITH_ERROR() \
    do { \
        auto error = SDL_GetError(); \
        std::stringstream error_message_stream; \
        error_message_stream << "Fatal error in " << __PRETTY_FUNCTION__ << ": " << error << "\nExiting..."; \
        std::string error_message = error_message_stream.str(); \
        std::cerr << error_message << "\n"; \
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", error_message.c_str(), NULL); \
        exit(1); \
    } while (false)

#define SDL_QUIT_IF_ERROR(result) \
    do { \
        if ((result) != 0) { \
            SDL_QUIT_WITH_ERROR(); \
        } \
    } while (false)

namespace sdl {
using window_t = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using window_surface_t = SDL_Surface*;

void init_sub_system(Uint32 flags) noexcept {
    auto result = SDL_InitSubSystem(flags);
    SDL_QUIT_IF_ERROR(result);
}

window_t create_window(const char* title, int x, int y, int w, int h, Uint32 flags) noexcept {
    auto window = SDL_CreateWindow(title, x, y, w, h, flags);

    if (window == nullptr) {
        SDL_QUIT_WITH_ERROR();
    }

    return window_t(window, SDL_DestroyWindow);
}

window_surface_t get_window_surface(const window_t& window) noexcept {
    auto window_surface = SDL_GetWindowSurface(window.get());

    if (window_surface == nullptr) {
        SDL_QUIT_WITH_ERROR();
    }

    return window_surface;
}

void update_window_surface(const window_t& window) noexcept {
    auto result = SDL_UpdateWindowSurface(window.get());
    SDL_QUIT_IF_ERROR(result);
}

struct pool_event_result {
    bool pending_event;
    SDL_Event event;
};

pool_event_result pool_event() noexcept {
    SDL_Event event;
    auto result = SDL_PollEvent(&event);

    return pool_event_result{!!result, event};
}

void fill_rect(window_surface_t surface, const SDL_Rect& rect, Uint32 color) noexcept {
    auto result = SDL_FillRect(surface, &rect, color);
    SDL_QUIT_IF_ERROR(result);
}

void gl_set_attribute(SDL_GLattr attribute, int value) noexcept {
    auto result = SDL_GL_SetAttribute(attribute, value);
    SDL_QUIT_IF_ERROR(result);
}

auto gl_delete_context(void* context) noexcept {
    SDL_GL_DeleteContext(context);
}

auto gl_create_context(const window_t& window) noexcept {
    auto gl_context = SDL_GL_CreateContext(window.get());

    if (gl_context == nullptr) {
        SDL_QUIT_WITH_ERROR();
    }

    return gl_context;
}

auto gl_try_use_vsync() noexcept {
    auto result = SDL_GL_SetSwapInterval(-1);
    if (result == 0) {
        return;
    }
    std::cerr << "Could not set adaptive vsync\n";

    result = SDL_GL_SetSwapInterval(1);
    if (result == 0) {
        return;
    }
    std::cerr << "Could not set vsync\n";
}

auto gl_swap_window(const window_t& window) noexcept {
    SDL_GL_SwapWindow(window.get());
}

auto get_performance_frequency() noexcept {
    return SDL_GetPerformanceFrequency();
}

auto get_performance_counter() noexcept {
    return SDL_GetPerformanceCounter();
}

void quit() noexcept {
    SDL_Quit();
}
}

#endif //SDL_HPP
