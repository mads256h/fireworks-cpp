//
// Created by mads on 14-01-25.
//

#ifndef GLEW_HPP
#define GLEW_HPP

#include <GL/glew.h>

#include <sstream>
#include <iostream>

#define GLEW_QUIT_WITH_ERROR(result) \
    do { \
        auto error = glewGetErrorString(result); \
        std::stringstream error_message_stream; \
        error_message_stream << "Fatal error in " << __PRETTY_FUNCTION__ << ": " << error << "\nExiting..."; \
        std::string error_message = error_message_stream.str(); \
        std::cerr << error_message << "\n"; \
        exit(1); \
    } while (false)

#define GLEW_QUIT_IF_ERROR(result) \
    do { \
        auto result_value = result; \
        if (result_value != GLEW_OK) { \
            GLEW_QUIT_WITH_ERROR(result_value); \
        } \
    } while (false)

namespace glew {
    void init() noexcept {
        glewExperimental = GL_TRUE;
        auto result = glewInit();
        GLEW_QUIT_IF_ERROR(result);
    }

}

#endif //GLEW_HPP
