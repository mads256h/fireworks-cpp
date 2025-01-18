// IWYU pragma: private, include "wrappers/opengl.hpp"

#ifndef FRAME_BUFFER_OBJECT_H
#define FRAME_BUFFER_OBJECT_H

#include <glm/glm.hpp>

#include <GL/glew.h>



namespace gl {
class [[nodiscard]] frame_buffer_object_t {
    GLuint m_frame_buffer_object;
    GLuint m_texture_object;
    bool m_moved;

    [[nodiscard]] constexpr explicit frame_buffer_object_t(GLuint frame_buffer_object, GLuint texture_object) noexcept
        : m_frame_buffer_object(frame_buffer_object), m_texture_object(texture_object), m_moved(false) {
    }

public:
    frame_buffer_object_t() = delete;

    frame_buffer_object_t(const frame_buffer_object_t&) = delete;

    [[nodiscard]] constexpr frame_buffer_object_t(frame_buffer_object_t&& other) noexcept
        : m_frame_buffer_object(other.m_frame_buffer_object), m_texture_object(other.m_frame_buffer_object), m_moved(false) {
        other.m_moved = true;
    }

    ~frame_buffer_object_t() noexcept {
        if (!m_moved) {
            glDeleteFramebuffers(1, &m_frame_buffer_object);
            glDeleteTextures(1, &m_texture_object);
        }
    }

    [[nodiscard]] constexpr GLuint texture_object() const noexcept {
        return m_texture_object;
    }

    void bind() const noexcept;

    void unbind() const noexcept;

    void set_size(const glm::ivec2& size) const noexcept;

    void bind_texture() const noexcept;

    [[nodiscard]] static frame_buffer_object_t create(const glm::ivec2& texture_size) noexcept;
};
}


#endif //FRAME_BUFFER_OBJECT_H
