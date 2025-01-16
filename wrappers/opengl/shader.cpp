//
// Created by mads on 16-01-25.
//

#include "shader.hpp"

#include "../opengl.hpp"

#include <cstdio>
#include <cstdlib>

GLuint gl::create_shader(GLenum type) noexcept {
    auto shader = glCreateShader(type);
    if (shader == 0) {
        // TODO: FIX
        fprintf(stderr, "Failed to create OpenGL shader\n");
        exit(EXIT_FAILURE);
    }

    return shader;
}

void gl::shader_source(GLuint shader, std::string_view source) noexcept {
    auto data = source.data();
    GLint size = source.length();
    glShaderSource(shader, 1, &data, &size);
}

void gl::compile_shader(GLuint shader) noexcept {
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE) {
        print_shader_info_log(shader);
        exit(EXIT_FAILURE);
    }
}

void gl::attach_shader(const gl::program_t& program, GLuint shader) noexcept {
    glAttachShader(program.value(), shader);
}

void gl::print_shader_info_log(GLuint shader) noexcept {
    GLint log_length;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

    std::vector<char> log(log_length);
    glGetShaderInfoLog(shader, log_length, &log_length, log.data());

    std::cerr << "Shader log:\n" << log.data() << std::endl;
}
