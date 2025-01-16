//
// Created by mads on 14-01-25.
//

#ifndef OPENGL_HPP
#define OPENGL_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <string_view>
#include <vector>
#include <array>
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

attribute_location_t get_attribute_location(const program_t& program, const char* name) noexcept;

void enable_vertex_attribute_array(const attribute_location_t& attribute) noexcept;

void disable_vertex_attribute_array(const attribute_location_t& attribute) noexcept;

template<typename TValue, GLenum Target, GLint Size, GLenum Type, bool Instanced = false, GLenum Usage = GL_STATIC_DRAW>
class attribute_buffer_object_t {
    attribute_location_t m_attribute_location;
    GLuint m_buffer_object;
    bool m_moved;

    explicit constexpr attribute_buffer_object_t(attribute_location_t attribute_location, GLuint buffer_object) noexcept
        : m_attribute_location(attribute_location), m_buffer_object(buffer_object), m_moved(false) {
    }

    explicit constexpr attribute_buffer_object_t(GLuint buffer_object) noexcept
        : m_attribute_location(0), m_buffer_object(buffer_object), m_moved(false) {
    }

public:
    attribute_buffer_object_t() = delete;


    constexpr attribute_buffer_object_t(attribute_buffer_object_t&& other) noexcept
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

    template<GLenum ETarget = Target, typename = std::enable_if_t<ETarget != GL_ELEMENT_ARRAY_BUFFER> >
    void upload() const noexcept {
        auto iterations = 1;

        // mat4 use 4 locations
        if constexpr (std::is_same_v<TValue, glm::mat4>) {
            iterations = 4;
        }

        // Set location data
        for (auto i = 0; i < iterations; i++) {
            const auto location = m_attribute_location.attribute_location() + i;
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location, Size, Type, GL_FALSE, sizeof(TValue),
                                  (const GLvoid*) (sizeof(GLfloat) * i * Size));
            if constexpr (Instanced) {
                glVertexAttribDivisor(location, 1);
            } else {
                glVertexAttribDivisor(location, 0);
            }
        }
    }

    template<GLenum ETarget, typename = std::enable_if_t<ETarget == GL_ELEMENT_ARRAY_BUFFER> >
    static attribute_buffer_object_t create_buffer_object() {
        GLuint buffer_object;
        glGenBuffers(1, &buffer_object);
        glBindBuffer(Target, buffer_object);

        return attribute_buffer_object_t(buffer_object);
    }

    template<typename TContainer, GLenum ETarget = Target, typename = std::enable_if_t<
        ETarget == GL_ELEMENT_ARRAY_BUFFER> >
    static attribute_buffer_object_t create_buffer_object(const TContainer& data) {
        static_assert(std::is_same_v<typename TContainer::value_type, TValue>);

        GLuint buffer_object;
        glGenBuffers(1, &buffer_object);
        glBindBuffer(Target, buffer_object);
        glBufferData(Target, data.size() * sizeof(TValue), data.data(), Usage);

        return attribute_buffer_object_t(buffer_object);
    }

    template<GLenum ETarget = Target, typename = std::enable_if_t<ETarget != GL_ELEMENT_ARRAY_BUFFER> >
    static attribute_buffer_object_t create_buffer_object(const program_t& program,
                                                          const char* attribute_name) noexcept {
        auto attribute_location = get_attribute_location(program, attribute_name);

        GLuint buffer_object;
        glGenBuffers(1, &buffer_object);
        glBindBuffer(Target, buffer_object);

        return attribute_buffer_object_t(attribute_location, buffer_object);
    }

    template<typename TContainer, GLenum ETarget = Target, typename = std::enable_if_t<
        ETarget != GL_ELEMENT_ARRAY_BUFFER> >
    static attribute_buffer_object_t create_buffer_object(const TContainer& data,
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

program_t create_program() noexcept;

void print_program_info_log(const program_t& program) noexcept;

void link_program(const program_t& program) noexcept;

void use_program(const program_t& program) noexcept;

void unbind_program() noexcept;

uniform_location_t get_uniform_location(const program_t& program, const char* name) noexcept;

void clear_color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) noexcept;

void clear(GLbitfield mask) noexcept;

vertex_array_object_t generate_vertex_array_object() noexcept;

void vertex_attrib_pointer(const attribute_location_t& attribute) noexcept;

void uniform_matrix(const uniform_location_t& uniform, const glm::mat4& matrix) noexcept;

void draw_elements(GLenum mode, GLsizei count, GLenum type) noexcept;
}

#endif //OPENGL_HPP
