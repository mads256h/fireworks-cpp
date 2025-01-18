//
// Created by mads on 16-01-25.
//

#include "opengl.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>
#include <vector>
#include <iostream>


void gl::enable(GLenum cap) noexcept {
    glEnable(cap);
}

void gl::disable(GLenum cap) noexcept {
    glDisable(cap);
}

void gl::debug_message_callback(GLDEBUGPROC callback, const void* userParameter) noexcept {
    glDebugMessageCallback(callback, userParameter);
}

gl::program_t gl::create_program() noexcept {
    auto program = glCreateProgram();
    if (program == 0) {
        std::cerr << "Failed to create OpenGL program" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return {program, [](auto program) { glDeleteProgram(program); }};
}

void gl::print_program_info_log(const program_t& program) noexcept {
    GLint log_length;

    glGetProgramiv(program.value(), GL_INFO_LOG_LENGTH, &log_length);

    std::vector<char> log(log_length);
    glGetProgramInfoLog(program.value(), log_length, &log_length, log.data());

    std::cerr << "Program log:\n" << log.data() << std::endl;
}

void gl::link_program(const program_t& program) noexcept {
    glLinkProgram(program.value());

    GLint status;
    glGetProgramiv(program.value(), GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        print_program_info_log(program);
        std::exit(EXIT_FAILURE);
    }
}

void gl::use_program(const program_t& program) noexcept {
    glUseProgram(program.value());
}

void gl::unbind_program() noexcept {
    glUseProgram(0);
}

gl::uniform_location_t gl::get_uniform_location(const program_t& program, const char* name) noexcept {
    auto uniform = glGetUniformLocation(program.value(), name);

    if (uniform == -1) {
        std::cerr << "Failed to get uniform location " << name << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return uniform_location_t(uniform);
}

void gl::clear_color(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) noexcept {
    glClearColor(red, green, blue, alpha);
}

void gl::clear(GLbitfield mask) noexcept {
    glClear(mask);
}

gl::vertex_array_object_t gl::generate_vertex_array_object() noexcept {
    GLuint vertex_array_object;
    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    return {vertex_array_object, [](GLuint vertex_array_object) { glDeleteVertexArrays(1, &vertex_array_object); }};
}

void gl::uniform_matrix(const uniform_location_t& uniform, const glm::mat4& matrix) noexcept {
    glUniformMatrix4fv(uniform.uniform_location(), 1, GL_FALSE, glm::value_ptr(matrix));
}

void gl::uniform_vec3(const uniform_location_t& uniform, const glm::vec3& vector) noexcept {
    glUniform3fv(uniform.uniform_location(), 1, glm::value_ptr(vector));
}

void gl::uniform_float(const uniform_location_t& uniform, float value) noexcept {
    glUniform1f(uniform.uniform_location(), value);
}

void gl::uniform_frame_buffer(const uniform_location_t& uniform, GLint texture_index) noexcept {
    glUniform1i(uniform.uniform_location(), texture_index);
}

void gl::draw_arrays(GLenum mode, GLint first, GLsizei count) noexcept {
    glDrawArrays(mode, first, count);
}

void gl::draw_elements(GLenum mode, GLsizei count, GLenum type) noexcept {
    glDrawElements(mode, count, type, nullptr);
}
