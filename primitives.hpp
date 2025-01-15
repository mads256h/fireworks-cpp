//
// Created by mads on 15-01-25.
//

#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include <glm/glm.hpp>

class line {
    glm::mat4 m_transform_matrix{};
    glm::vec3 m_color;
    float m_start_width;
    float m_end_width;

public:
    line(glm::vec2 start_position, glm::vec2 end_position, glm::vec3 color, float start_width, float end_width) noexcept;

    [[nodiscard]] constexpr const glm::mat4& transform_matrix() const noexcept { return m_transform_matrix; }
    [[nodiscard]] constexpr const glm::vec3& color() const noexcept { return m_color; }
    [[nodiscard]] constexpr float start_width() const noexcept { return m_start_width; }
    [[nodiscard]] constexpr float end_width() const noexcept { return m_end_width; }
};

#endif //PRIMITIVES_HPP
