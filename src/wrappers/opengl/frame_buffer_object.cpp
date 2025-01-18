//
// Created by mads on 18-01-25.
//

#include "frame_buffer_object.hpp"

void set_texture_size(GLuint texture_object, const glm::ivec2& size) {
    glBindTexture(GL_TEXTURE_2D, texture_object);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void gl::frame_buffer_object_t::bind() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_object);
}

void gl::frame_buffer_object_t::unbind() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gl::frame_buffer_object_t::set_size(const glm::ivec2& size) const noexcept {
    set_texture_size(m_texture_object, size);
}

void gl::frame_buffer_object_t::bind_texture() const noexcept {
    glBindTexture(GL_TEXTURE_2D, m_texture_object);
}


gl::frame_buffer_object_t gl::frame_buffer_object_t::create(const glm::ivec2& texture_size) noexcept {
    GLuint frame_buffer_object;
    glGenFramebuffers(1, &frame_buffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    GLuint texture_object;
    glGenTextures(1, &texture_object);
    set_texture_size(texture_object, texture_size);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_object, 0);

    return frame_buffer_object_t{frame_buffer_object, texture_object};
}
