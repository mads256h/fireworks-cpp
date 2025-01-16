//
// Created by mads on 16-01-25.
//

#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>

#include <string_view>
#include <iostream>

#include "../../utilities.hpp"

namespace gl {
using program_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;

GLuint create_shader(GLenum type) noexcept;
void shader_source(GLuint shader, std::string_view source) noexcept;
void print_shader_info_log(GLuint shader) noexcept;
void compile_shader(GLuint shader) noexcept;
void attach_shader(const gl::program_t& program, GLuint shader) noexcept;

template<GLenum Type>
class shader_t {
    GLuint m_shader;
    bool m_moved;

    explicit constexpr shader_t(GLuint shader) noexcept: m_shader(shader), m_moved(false) {}

    public:
    shader_t() = delete;

    constexpr shader_t(shader_t&& other) noexcept: m_shader(other.m_shader), m_moved(false) {
        other.m_moved = true;
    }

    ~shader_t() noexcept {
        if (!m_moved) {
            std::cerr << "Deleted shader" << std::endl;
            glDeleteShader(m_shader);
        }
    }

    static shader_t create_shader(const program_t& program, std::string_view source) noexcept {
        auto shader = gl::create_shader(Type);
        shader_source(shader, source);
        compile_shader(shader);
        attach_shader(program, shader);

        return shader_t(shader);
    }
};

using vertex_shader_t = shader_t<GL_VERTEX_SHADER>;
using fragment_shader_t = shader_t<GL_FRAGMENT_SHADER>;
}



#endif //SHADER_HPP
