#ifndef SDL_HPP
#define SDL_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_messagebox.h>

#include <string>
#include <sstream>
#include <iostream>
#include <memory>

#include "../utilities.hpp"

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
using opengl_context_t = utilities::raii_wrapper<SDL_GLContext, decltype(&SDL_GL_DeleteContext)>;

struct pool_event_result {
    bool pending_event;
    SDL_Event event;
};

void init_sub_system(Uint32 flags) noexcept;

window_t create_window(const char* title, int x, int y, int w, int h, Uint32 flags) noexcept;

window_surface_t get_window_surface(const window_t& window) noexcept;

void update_window_surface(const window_t& window) noexcept;

pool_event_result pool_event() noexcept;

void fill_rect(window_surface_t surface, const SDL_Rect& rect, Uint32 color) noexcept;

void gl_set_attribute(SDL_GLattr attribute, int value) noexcept;

void gl_delete_context(void* context) noexcept;

opengl_context_t gl_create_context(const window_t& window) noexcept;

void gl_try_use_vsync() noexcept;

void gl_swap_window(const window_t& window) noexcept;

Uint64 get_performance_frequency() noexcept;

Uint64 get_performance_counter() noexcept;

void quit() noexcept;
}

#endif //SDL_HPP
