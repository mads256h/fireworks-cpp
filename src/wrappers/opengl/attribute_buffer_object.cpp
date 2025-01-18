//
// Created by mads on 18-01-25.
//

#include "attribute_buffer_object.hpp"

#include <cstdlib>

gl::attribute_location_t gl::get_attribute_location(const program_t& program, const char* name) noexcept {
    auto attribute = glGetAttribLocation(program.value(), name);

    if (attribute == -1) {
        std::cerr << "Failed to get attribute location " << name << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return attribute_location_t(attribute);
}

void gl::enable_vertex_attribute_array(const attribute_location_t& attribute) noexcept {
    glEnableVertexAttribArray(attribute.attribute_location());
}

void gl::disable_vertex_attribute_array(const attribute_location_t& attribute) noexcept {
    glDisableVertexAttribArray(attribute.attribute_location());
}

void gl::vertex_attrib_pointer(const attribute_location_t& attribute) noexcept {
    glVertexAttribPointer(attribute.attribute_location(), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
}
