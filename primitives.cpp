//
// Created by mads on 15-01-25.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // IWYU pragma: keep

#include "primitives.hpp"

#include <numbers>
#include <cmath>

glm::mat4 make_model_matrix(const glm::vec2& position, const float rotations_radians, const glm::vec2& scale) noexcept {
    auto model_matrix = glm::identity<glm::mat4>();
    model_matrix = glm::translate(model_matrix, glm::vec3(position.x, position.y, 0.0f));
    model_matrix = glm::rotate(model_matrix, rotations_radians, glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix = glm::scale(model_matrix, glm::vec3(scale.x, scale.y, 1.0f));

    return model_matrix;
}

line::line(glm::vec2 start_position,
           glm::vec2 end_position,
           glm::vec3 color,
           float start_width,
           float end_width) noexcept
    : m_color(color),
      m_start_width(start_width),
      m_end_width(end_width) {
    constexpr auto width = 50.0f;

    auto vector = end_position - start_position;
    auto length = glm::length(vector);
    auto center_length = length / 2.0f;
    vector = glm::normalize(vector) * center_length;

    auto position = start_position + vector;
    auto rotation = std::atan2(start_position.y - end_position.y, start_position.x - end_position.x) +
                    std::numbers::pi_v<float> / 2.0f;
    auto scale = glm::vec2(width, length);

    m_transform_matrix = make_model_matrix(position, rotation, scale);
}
