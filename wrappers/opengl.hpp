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

#include "../utilities.hpp"

namespace gl {
    using program_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;

    class attribute_location_t {
        GLint m_attribute_location;

    public:
        constexpr explicit attribute_location_t(GLint location) noexcept : m_attribute_location(location) {}
        [[nodiscard]] constexpr GLint attribute_location() const noexcept { return m_attribute_location; }
    };

    class uniform_location_t {
        GLint m_uniform_location;

    public:
        constexpr explicit uniform_location_t(GLint location) noexcept : m_uniform_location(location) {}
        [[nodiscard]] constexpr GLint uniform_location() const noexcept { return m_uniform_location; }
    };

    auto get_attribute_location(const program_t& program, const char* name) noexcept {
        auto attribute = glGetAttribLocation(program.value(), name);

        if (attribute == -1) {
            std::cerr << "Failed to get attribute location " << name << std::endl;
            //exit(EXIT_FAILURE);
        }

        return attribute_location_t(attribute);
    }

    auto enable_vertex_attribute_array(const attribute_location_t& attribute) noexcept {
        glEnableVertexAttribArray(attribute.attribute_location());
    }

    auto disable_vertex_attribute_array(const attribute_location_t& attribute) noexcept {
        glDisableVertexAttribArray(attribute.attribute_location());
    }

    template<typename TValue, GLenum Target, GLint Size, GLenum Type, bool Instanced = false, GLenum Usage = GL_STATIC_DRAW>
    class attribute_buffer_object_t {
        attribute_location_t m_attribute_location;
        GLuint m_buffer_object;
        bool m_moved;

        explicit constexpr attribute_buffer_object_t(attribute_location_t attribute_location, GLuint buffer_object) noexcept
            : m_attribute_location(attribute_location), m_buffer_object(buffer_object), m_moved(false) {}

        explicit constexpr attribute_buffer_object_t(GLuint buffer_object) noexcept
            : m_attribute_location(0), m_buffer_object(buffer_object), m_moved(false) {}


    public:
        attribute_buffer_object_t() = delete;


        constexpr attribute_buffer_object_t(attribute_buffer_object_t&& other) noexcept: m_attribute_location(other.m_attribute_location), m_buffer_object(other.m_buffer_object), m_moved(false) {
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

        template<GLenum ETarget = Target, typename = std::enable_if_t<ETarget != GL_ELEMENT_ARRAY_BUFFER>>
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
                glVertexAttribPointer(location, Size, Type, GL_FALSE, sizeof(TValue), (const GLvoid*)(sizeof(GLfloat) * i * Size));
                if constexpr (Instanced) {
                    glVertexAttribDivisor(location, 1);
                }
                else {
                    glVertexAttribDivisor(location, 0);
                }
            }
        }

        template<GLenum ETarget, typename = std::enable_if_t<ETarget == GL_ELEMENT_ARRAY_BUFFER>>
        static attribute_buffer_object_t create_buffer_object() {
            GLuint buffer_object;
            glGenBuffers(1, &buffer_object);
            glBindBuffer(Target, buffer_object);

            return attribute_buffer_object_t(buffer_object);

        }

        template<typename TContainer, GLenum ETarget = Target, typename = std::enable_if_t<ETarget == GL_ELEMENT_ARRAY_BUFFER>>
        static attribute_buffer_object_t create_buffer_object(const TContainer& data) {
            static_assert(std::is_same_v<typename TContainer::value_type, TValue>);

            GLuint buffer_object;
            glGenBuffers(1, &buffer_object);
            glBindBuffer(Target, buffer_object);
            glBufferData(Target, data.size() * sizeof(TValue), data.data(), Usage);

            return attribute_buffer_object_t(buffer_object);

        }

        template<GLenum ETarget = Target, typename = std::enable_if_t<ETarget != GL_ELEMENT_ARRAY_BUFFER>>
        static attribute_buffer_object_t create_buffer_object(const program_t& program, const char* attribute_name) noexcept {
            auto attribute_location = get_attribute_location(program, attribute_name);

            GLuint buffer_object;
            glGenBuffers(1, &buffer_object);
            glBindBuffer(Target, buffer_object);

            return attribute_buffer_object_t(attribute_location, buffer_object);
        }

        template<typename TContainer, GLenum ETarget = Target, typename = std::enable_if_t<ETarget != GL_ELEMENT_ARRAY_BUFFER>>
        static attribute_buffer_object_t create_buffer_object(const TContainer& data, const program_t& program, const char* attribute_name) noexcept {
            static_assert(std::is_same_v<typename TContainer::value_type, TValue>);

            auto attribute_location = get_attribute_location(program, attribute_name);

            GLuint buffer_object;
            glGenBuffers(1, &buffer_object);
            glBindBuffer(Target, buffer_object);
            glBufferData(Target, data.size() * sizeof(TValue), data.data(), Usage);

            return attribute_buffer_object_t(attribute_location, buffer_object);
        }
    };

    using shader_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;
    using vertex_array_object_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;
    using vertex_buffer_object_t = attribute_buffer_object_t<glm::vec2, GL_ARRAY_BUFFER, 2, GL_FLOAT>;
    using index_buffer_object_t = attribute_buffer_object_t<GLuint, GL_ELEMENT_ARRAY_BUFFER, -1, GL_UNSIGNED_INT>;
    using texture_coordinate_buffer_object_t = attribute_buffer_object_t<glm::vec2, GL_ARRAY_BUFFER, 2, GL_FLOAT>;
    using model_matrix_buffer_object_t = attribute_buffer_object_t<glm::mat4, GL_ARRAY_BUFFER, 4, GL_FLOAT, true, GL_DYNAMIC_DRAW>;
    using model_color_buffer_object_t = attribute_buffer_object_t<glm::vec3, GL_ARRAY_BUFFER, 3, GL_FLOAT, true, GL_DYNAMIC_DRAW>;
    using vertex_width_buffer_object_t = attribute_buffer_object_t<glm::vec2, GL_ARRAY_BUFFER, 2, GL_FLOAT, true, GL_DYNAMIC_DRAW>;



    void enable(GLenum cap) noexcept {
        glEnable(cap);
    }

    void debug_message_callback(GLDEBUGPROC callback, const void* userParameter) noexcept {
        glDebugMessageCallback(callback, userParameter);
    }

    program_t create_program() noexcept {
        auto program = glCreateProgram();
        if (program == 0) {
            // TODO: FIX
            fprintf(stderr, "Failed to create OpenGL program\n");
            exit(EXIT_FAILURE);
        }

        return program_t(program, [](auto program) { glDeleteProgram(program); });
    }

    shader_t create_shader(GLenum type) noexcept {
        auto shader = glCreateShader(type);
        if (shader == 0) {
            // TODO: FIX
            fprintf(stderr, "Failed to create OpenGL shader\n");
            exit(EXIT_FAILURE);
        }


        return shader_t(shader, [](auto shader) { glDeleteShader(shader); });
    }

    auto shader_source(const shader_t& shader, const GLchar* source) noexcept {
        glShaderSource(shader.value(), 1, &source, nullptr);
    }

    auto print_shader_info_log(const shader_t& shader) noexcept {
        GLint log_length;

        glGetShaderiv(shader.value(), GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> log(log_length);
        glGetShaderInfoLog(shader.value(), log_length, &log_length, log.data());

        std::cerr << "Shader log:\n" << log.data() << std::endl;
    }

    auto print_program_info_log(const program_t& program) noexcept {
        GLint log_length;

        glGetProgramiv(program.value(), GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> log(log_length);
        glGetProgramInfoLog(program.value(), log_length, &log_length, log.data());

        std::cerr << "Program log:\n" << log.data() << std::endl;
    }

    auto compile_shader(const shader_t& shader) noexcept {
        glCompileShader(shader.value());

        GLint status = GL_FALSE;
        glGetShaderiv(shader.value(), GL_COMPILE_STATUS, &status);

        if (status != GL_TRUE) {
            print_shader_info_log(shader);
            exit(EXIT_FAILURE);
        }
    }

    auto attach_shader(const program_t& program, const shader_t& shader) noexcept {
        glAttachShader(program.value(), shader.value());
    }

    auto link_program(const program_t& program) noexcept {
        glLinkProgram(program.value());

        GLint status;
        glGetProgramiv(program.value(), GL_LINK_STATUS, &status);

        if (status != GL_TRUE) {
            print_program_info_log(program);
            exit(EXIT_FAILURE);
        }
    }

    auto use_program(const program_t& program) noexcept {
        glUseProgram(program.value());
    }

    auto unbind_program() noexcept {
        glUseProgram(0);
    }

    auto get_uniform_location(const program_t& program, const char* name) noexcept {
        auto uniform = glGetUniformLocation(program.value(), name);

        if (uniform == -1) {
            std::cerr << "Failed to get uniform location " << name << std::endl;
            //exit(EXIT_FAILURE);
        }

        return uniform_location_t(uniform);
    }

    auto clear_color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) noexcept {
        glClearColor(red, green, blue, alpha);
    }

    auto clear(GLbitfield mask) noexcept {
        glClear(mask);
    }

    template<typename TContainer, GLenum Target, GLenum Usage = GL_STATIC_DRAW, typename TValue = typename TContainer::value_type>
    auto generate_buffer_object(const TContainer& data) noexcept {
        GLuint buffer_object;
        glGenBuffers(1, &buffer_object);
        glBindBuffer(Target, buffer_object);
        glBufferData(Target, data.size() * sizeof(TValue), data.data(), Usage);
    }

    auto generate_vertex_array_object() noexcept {
        GLuint vertex_array_object;
        glGenVertexArrays(1, &vertex_array_object);
        glBindVertexArray(vertex_array_object);

        return vertex_array_object_t(vertex_array_object, [](GLuint vertex_array_object) {glDeleteVertexArrays(1, &vertex_array_object); });
    }

    template<typename TContainer, typename TValue = typename TContainer::value_type>
    auto generate_texture_coordinate_buffer_object(const TContainer& texture_coordinate_data) noexcept {
        GLuint texture_coordinate_buffer_object;
        glGenBuffers(1, &texture_coordinate_buffer_object);
        glBindBuffer(GL_ARRAY_BUFFER, texture_coordinate_buffer_object);
        glBufferData(GL_ARRAY_BUFFER, texture_coordinate_data.size() * sizeof(TValue), texture_coordinate_data.data(), GL_STATIC_DRAW);

        return texture_coordinate_buffer_object_t(std::move(texture_coordinate_buffer_object), [](GLuint texture_coordinate_buffer) {glDeleteBuffers(1, &texture_coordinate_buffer); });
    }

    auto vertex_attrib_pointer(const attribute_location_t& attribute) noexcept {
        glVertexAttribPointer(attribute.attribute_location(), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    }

    auto uniform_matrix(const uniform_location_t& uniform, const glm::mat4& matrix) noexcept {
        glUniformMatrix4fv(uniform.uniform_location(), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    auto draw_elements(GLenum mode, GLsizei count, GLenum type) noexcept {
        glDrawElements(mode, count, type, nullptr);
    }
}

#endif //OPENGL_HPP
