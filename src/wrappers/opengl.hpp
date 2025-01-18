//
// Created by mads on 14-01-25.
//

#ifndef OPENGL_HPP
#define OPENGL_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <type_traits>
#include <iostream>

#include "opengl/shader.hpp"
#include "../utilities.hpp"

namespace gl {
class attribute_location_t {
    GLint m_attribute_location;

public:
    constexpr explicit attribute_location_t(GLint location) noexcept
        : m_attribute_location(location) {
    }

    [[nodiscard]] constexpr GLint attribute_location() const noexcept { return m_attribute_location; }
};

class uniform_location_t {
    GLint m_uniform_location;

public:
    constexpr explicit uniform_location_t(GLint location) noexcept
        : m_uniform_location(location) {
    }

    [[nodiscard]] constexpr GLint uniform_location() const noexcept { return m_uniform_location; }
};

[[nodiscard]] attribute_location_t get_attribute_location(const program_t& program, const char* name) noexcept;

void enable_vertex_attribute_array(const attribute_location_t& attribute) noexcept;

void disable_vertex_attribute_array(const attribute_location_t& attribute) noexcept;

template<typename TValue, GLenum Target, GLint Size, GLenum Type, bool Instanced = false, GLenum Usage = GL_STATIC_DRAW>
class [[nodiscard]] attribute_buffer_object_t {
    attribute_location_t m_attribute_location;
    GLuint m_buffer_object;
    bool m_moved;

    [[nodiscard]] explicit constexpr attribute_buffer_object_t(attribute_location_t attribute_location, GLuint buffer_object) noexcept
        : m_attribute_location(attribute_location), m_buffer_object(buffer_object), m_moved(false) {
    }

    [[nodiscard]] explicit constexpr attribute_buffer_object_t(GLuint buffer_object) noexcept
        : m_attribute_location(0), m_buffer_object(buffer_object), m_moved(false) {
    }

public:
    attribute_buffer_object_t() = delete;


    [[nodiscard]] constexpr attribute_buffer_object_t(attribute_buffer_object_t&& other) noexcept
        : m_attribute_location(other.m_attribute_location), m_buffer_object(other.m_buffer_object), m_moved(false) {
        other.m_moved = true;
    }

    ~attribute_buffer_object_t() {
        if (!m_moved) {
            std::cerr << "Deleted buffer object" << std::endl;
            glDeleteBuffers(1, &m_buffer_object);
        }
    }

    void bind() const noexcept {
        glBindBuffer(Target, m_buffer_object);
    }

    template<typename TContainer>
    void set_data(const TContainer& data) const noexcept {
        static_assert(std::is_same_v<typename TContainer::value_type, TValue>);

        bind();
        glBufferData(Target, data.size() * sizeof(TValue), data.data(), Usage);
    }

    template<GLenum ETarget = Target, typename = std::enable_if_t<ETarget != GL_ELEMENT_ARRAY_BUFFER>, int Iterations =
            std::is_same_v<TValue, glm::mat4> ? 4 : 1>
    void upload() const noexcept {
        // Set location data
        for (auto i = 0; i < Iterations; i++) {
            const auto location = m_attribute_location.attribute_location() + i;
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location, Size, Type, GL_FALSE, sizeof(TValue),
                                  reinterpret_cast<const GLvoid*>(sizeof(GLfloat) * i * Size));
            if constexpr (Instanced) {
                glVertexAttribDivisor(location, 1);
            } else {
                glVertexAttribDivisor(location, 0);
            }
        }
    }

    template<GLenum ETarget, typename = std::enable_if_t<ETarget == GL_ELEMENT_ARRAY_BUFFER> >
    [[nodiscard]] static attribute_buffer_object_t create_buffer_object() {
        GLuint buffer_object;
        glGenBuffers(1, &buffer_object);
        glBindBuffer(Target, buffer_object);

        return attribute_buffer_object_t(buffer_object);
    }

    template<typename TContainer, GLenum ETarget = Target, typename = std::enable_if_t<
        ETarget == GL_ELEMENT_ARRAY_BUFFER> >
    [[nodiscard]] static attribute_buffer_object_t create_buffer_object(const TContainer& data) {
        static_assert(std::is_same_v<typename TContainer::value_type, TValue>);

        GLuint buffer_object;
        glGenBuffers(1, &buffer_object);
        glBindBuffer(Target, buffer_object);
        glBufferData(Target, data.size() * sizeof(TValue), data.data(), Usage);

        return attribute_buffer_object_t(buffer_object);
    }

    template<GLenum ETarget = Target, typename = std::enable_if_t<ETarget != GL_ELEMENT_ARRAY_BUFFER> >
    [[nodiscard]] static attribute_buffer_object_t create_buffer_object(const program_t& program,
                                                          const char* attribute_name) noexcept {
        auto attribute_location = get_attribute_location(program, attribute_name);

        GLuint buffer_object;
        glGenBuffers(1, &buffer_object);
        glBindBuffer(Target, buffer_object);

        return attribute_buffer_object_t(attribute_location, buffer_object);
    }

    template<typename TContainer, GLenum ETarget = Target, typename = std::enable_if_t<
        ETarget != GL_ELEMENT_ARRAY_BUFFER> >
    [[nodiscard]] static attribute_buffer_object_t create_buffer_object(const TContainer& data,
                                                          const program_t& program,
                                                          const char* attribute_name) noexcept {
        static_assert(std::is_same_v<typename TContainer::value_type, TValue>);

        auto attribute_location = get_attribute_location(program, attribute_name);

        GLuint buffer_object;
        glGenBuffers(1, &buffer_object);
        glBindBuffer(Target, buffer_object);
        glBufferData(Target, data.size() * sizeof(TValue), data.data(), Usage);

        return attribute_buffer_object_t(attribute_location, buffer_object);
    }
};


using vertex_array_object_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;
using vertex_buffer_object_t = attribute_buffer_object_t<glm::vec2, GL_ARRAY_BUFFER, 2, GL_FLOAT>;
using index_buffer_object_t = attribute_buffer_object_t<GLuint, GL_ELEMENT_ARRAY_BUFFER, -1, GL_UNSIGNED_INT>;
using texture_coordinate_buffer_object_t = attribute_buffer_object_t<glm::vec2, GL_ARRAY_BUFFER, 2, GL_FLOAT>;
using model_matrix_buffer_object_t = attribute_buffer_object_t<glm::mat4, GL_ARRAY_BUFFER, 4, GL_FLOAT, true,
    GL_DYNAMIC_DRAW>;
using model_color_buffer_object_t = attribute_buffer_object_t<glm::vec3, GL_ARRAY_BUFFER, 3, GL_FLOAT, true,
    GL_DYNAMIC_DRAW>;
using vertex_width_buffer_object_t = attribute_buffer_object_t<glm::vec2, GL_ARRAY_BUFFER, 2, GL_FLOAT, true,
    GL_DYNAMIC_DRAW>;


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

void vertex_attrib_pointer(const attribute_location_t& attribute) noexcept;

void uniform_matrix(const uniform_location_t& uniform, const glm::mat4& matrix) noexcept;

void uniform_vec3(const uniform_location_t& uniform, const glm::vec3& vector) noexcept;

void uniform_float(const uniform_location_t& uniform, float value) noexcept;

void draw_arrays(GLenum mode, GLint first, GLsizei count) noexcept;

void draw_elements(GLenum mode, GLsizei count, GLenum type) noexcept;
}

#endif //OPENGL_HPP
