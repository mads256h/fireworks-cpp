//
// Created by mads on 16-01-25.
//

#include "glew.hpp"

#include <GL/glew.h>

#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define GLEW_QUIT_WITH_ERROR(result) \
    do { \
        auto error = glewGetErrorString(result); \
        std::stringstream error_message_stream; \
        error_message_stream << "Fatal error in " << __PRETTY_FUNCTION__ << ": " << error << "\nExiting..."; \
        std::string error_message = error_message_stream.str(); \
        std::cerr << error_message << "\n"; \
        std::exit(1); \
    } while (false)

#define GLEW_QUIT_IF_ERROR(result) \
    do { \
        auto result_value = result; \
        if (result_value != GLEW_OK) { \
            GLEW_QUIT_WITH_ERROR(result_value); \
        } \
    } while (false)

void glew::init() noexcept {
    glewExperimental = GL_TRUE;
    auto result = glewInit();
    GLEW_QUIT_IF_ERROR(result);
}
