//
// Created by mads on 14-01-25.
//

#ifndef OPENGL_HPP
#define OPENGL_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "opengl/shader.hpp"
#include "../utilities.hpp"

namespace gl {

class uniform_location_t {
    GLint m_uniform_location;

public:
    constexpr explicit uniform_location_t(GLint location) noexcept
        : m_uniform_location(location) {
    }

    [[nodiscard]] constexpr GLint uniform_location() const noexcept { return m_uniform_location; }
};


using vertex_array_object_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;


void enable(GLenum cap) noexcept;

void disable(GLenum cap) noexcept;

void debug_message_callback(GLDEBUGPROC callback, const void* userParameter) noexcept;

[[nodiscard]] program_t create_program() noexcept;

void print_program_info_log(const program_t& program) noexcept;

void link_program(const program_t& program) noexcept;

void use_program(const program_t& program) noexcept;

void unbind_program() noexcept;

[[nodiscard]] uniform_location_t get_uniform_location(const program_t& program, const char* name) noexcept;

void clear_color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) noexcept;

void clear(GLbitfield mask) noexcept;

[[nodiscard]] vertex_array_object_t generate_vertex_array_object() noexcept;


void uniform_matrix(const uniform_location_t& uniform, const glm::mat4& matrix) noexcept;

void uniform_vec3(const uniform_location_t& uniform, const glm::vec3& vector) noexcept;

void uniform_float(const uniform_location_t& uniform, float value) noexcept;

void uniform_frame_buffer(const uniform_location_t& uniform, GLint texture_index) noexcept;

void draw_arrays(GLenum mode, GLint first, GLsizei count) noexcept;

void draw_elements(GLenum mode, GLsizei count, GLenum type) noexcept;
}

#endif //OPENGL_HPP
