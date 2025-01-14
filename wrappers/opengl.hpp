//
// Created by mads on 14-01-25.
//

#ifndef OPENGL_HPP
#define OPENGL_HPP

#include <GL/glew.h>

#include <string_view>
#include <vector>
#include <array>

#include "../utilities.hpp"

namespace gl {
    using program_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;
    using shader_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;
    using vertex_array_object_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;
    using vertex_buffer_object_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;
    using index_buffer_object_t = utilities::raii_wrapper<GLuint, void(*)(GLuint)>;

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

        return program_t(std::move(program), [](auto program) { glDeleteProgram(program); });
    }

    shader_t create_shader(GLenum type) noexcept {
        auto shader = glCreateShader(type);
        if (shader == 0) {
            // TODO: FIX
            fprintf(stderr, "Failed to create OpenGL shader\n");
            exit(EXIT_FAILURE);
        }


        return shader_t(std::move(shader), [](auto shader) { glDeleteShader(shader); });
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

    auto get_attribute_location(const program_t& program, const char* name) noexcept {
        auto attribute = glGetAttribLocation(program.value(), name);

        if (attribute == -1) {
            std::cerr << "Failed to get attribute location " << name << std::endl;
            exit(EXIT_FAILURE);
        }

        return attribute;
    }

    auto get_uniform_location(const program_t& program, const char* name) noexcept {
        auto uniform = glGetUniformLocation(program.value(), name);

        if (uniform == -1) {
            std::cerr << "Failed to get uniform location " << name << std::endl;
            //exit(EXIT_FAILURE);
        }

        return uniform;
    }

    auto clear_color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) noexcept {
        glClearColor(red, green, blue, alpha);
    }

    auto clear(GLbitfield mask) noexcept {
        glClear(mask);
    }

    auto bind_vertex_buffer_object(const vertex_buffer_object_t& buffer_object) noexcept {
        glBindBuffer(GL_ARRAY_BUFFER, buffer_object.value());
    }

    auto generate_vertex_buffer_object(const std::vector<GLfloat>& vertex_data) noexcept {

        auto leaked_data = new GLfloat[vertex_data.size()];
        std::copy(vertex_data.begin(), vertex_data.end(), leaked_data);

        GLuint vertex_buffer_object;
        glGenBuffers(1, &vertex_buffer_object);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(GLfloat), leaked_data, GL_STATIC_DRAW);

        return vertex_buffer_object_t(std::move(vertex_buffer_object), [](GLuint vertex_buffer_object) { glDeleteBuffers(1, &vertex_buffer_object); });
    }

    auto bind_index_buffer_object(const index_buffer_object_t& buffer_object) noexcept {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_object.value());
    }

    auto generate_index_buffer_object(const std::vector<GLuint>& index_data) noexcept {
        GLuint index_buffer_object;
        glGenBuffers(1, &index_buffer_object);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.size() * sizeof(GLuint), index_data.data(), GL_STATIC_DRAW);

        return index_buffer_object_t(std::move(index_buffer_object), [](GLuint index_buffer_object) { glDeleteBuffers(1, &index_buffer_object); });
    }

    auto generate_vertex_array_object() noexcept {
        GLuint vertex_array_object;
        glGenVertexArrays(1, &vertex_array_object);
        glBindVertexArray(vertex_array_object);

        return vertex_array_object_t(std::move(vertex_array_object), [](GLuint vertex_array_object) {glDeleteVertexArrays(1, &vertex_array_object); });
    }

    auto enable_vertex_attribute_array(GLint attribute) noexcept {
        glEnableVertexAttribArray(attribute);
    }

    auto disable_vertex_attribute_array(GLint attribute) noexcept {
        glDisableVertexAttribArray(attribute);
    }

    auto vertex_attrib_pointer(GLuint index) noexcept {
        glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    }

    auto draw_elements(GLenum mode, GLsizei count, GLenum type) noexcept {
        glDrawElements(mode, count, type, nullptr);
    }
}

#endif //OPENGL_HPP
