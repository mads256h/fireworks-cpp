#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "wrappers/sdl.hpp"
#include "wrappers/glew.hpp"
#include "wrappers/opengl.hpp"

#include "primitives.hpp"

#include "generated/resources.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

constexpr std::array<glm::vec2, 4> vertex_positions = {
    glm::vec2{-0.5f, -0.5f},
     {0.5f, -0.5f},
     {0.5f,  0.5f},
    {-0.5f,  0.5f}
};

constexpr std::array<glm::vec2, 4> vertex_uvs = {
    glm::vec2{0.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f},
    {0.0f, 1.0f}
};

constexpr std::array<GLuint, 4> vertex_indices = {0, 1, 2, 3};

struct shader_stuff {
    gl::program_t program;
    gl::shader_t vertex_shader;
    gl::shader_t fragment_shader;
    gl::uniform_location_t projection_uniform;
    gl::vertex_array_object_t vertex_array_object;
    gl::vertex_buffer_object_t vertex_buffer_object;
    gl::index_buffer_object_t index_buffer_object;
    gl::texture_coordinate_buffer_object_t texture_coordinate_buffer_object;
    gl::model_matrix_buffer_object_t model_matrix_buffer_object;
    gl::model_color_buffer_object_t model_color_buffer_object;
    gl::vertex_width_buffer_object_t vertex_width_buffer_object;
};
shader_stuff init_gl();
void render(const shader_stuff& stuff, const glm::mat4& projection_matrix, float delta_time, const std::vector<line>& lines);

void GLAPIENTRY debug_message_callback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void*);

int main() {
    sdl::init_sub_system(SDL_INIT_TIMER);
    sdl::init_sub_system(SDL_INIT_VIDEO);
    sdl::init_sub_system(SDL_INIT_EVENTS);

    {
        std::vector<line> lines;
        auto quit = false;
        bool got_first_point = false;
        glm::vec2 first_point = glm::vec2{0.0f};

        const auto performance_frequency = static_cast<double>(sdl::get_performance_frequency());
        auto delta_time = 0.016f; // 1 frame at 60 fps initially.
        auto last_time = static_cast<float>(static_cast<double>(sdl::get_performance_counter()) / performance_frequency);

        sdl::gl_set_attribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        sdl::gl_set_attribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        sdl::gl_set_attribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        sdl::gl_set_attribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        sdl::gl_set_attribute(SDL_GL_MULTISAMPLESAMPLES,8);

        auto window = sdl::create_window("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 200, 200, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        auto gl_context = sdl::gl_create_context(window);
        glew::init();

        // Use vsync
        sdl::gl_try_use_vsync();

        auto stuff = init_gl();

        auto projection_matrix = glm::identity<glm::mat4>();

        while (!quit) {
            for (auto pool_event_result = sdl::pool_event(); pool_event_result.pending_event; pool_event_result = sdl::pool_event()) {
                switch (pool_event_result.event.type) {
                    case SDL_QUIT:
                        std::cout << "Quitting..." << std::endl;
                        quit = true;
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                        if (pool_event_result.event.button.button == SDL_BUTTON_LEFT) {
                            auto x = pool_event_result.event.button.x;
                            auto y = pool_event_result.event.button.y;
                            if (!got_first_point) {
                                first_point = glm::vec2{x, y};
                                got_first_point = true;
                            }
                            else {
                                glm::vec2 end_position{x, y};
                                lines.push_back({first_point, end_position, glm::vec3(1.0f), 50.0f, 50.0f});
                                got_first_point = false;
                            }
                            std::cout << "button pressed at (" << x << ", " << y << ")" << std::endl;
                        }
                        break;

                    case SDL_WINDOWEVENT:
                        if (pool_event_result.event.window.event == SDL_WINDOWEVENT_RESIZED) {
                            auto x = pool_event_result.event.window.data1;
                            auto y = pool_event_result.event.window.data2;

                            std::cout << "window size: " << x << ", " << y << std::endl;

                            projection_matrix = glm::ortho(0.0f, static_cast<float>(x), static_cast<float>(y), 0.0f, -1.0f, 1.0f);
                            glViewport(0, 0, x, y);
                        }
                    break;
                }
            }

            render(stuff, projection_matrix, delta_time, lines);

            auto now = static_cast<float>(static_cast<double>(sdl::get_performance_counter()) / performance_frequency);
            delta_time = now - last_time;
            last_time = now;

            sdl::gl_swap_window(window);
        }
    }

    sdl::quit();

    return EXIT_SUCCESS;
}

 shader_stuff init_gl() {
    gl::enable(GL_DEBUG_OUTPUT);
    gl::debug_message_callback(debug_message_callback, nullptr);

    gl::enable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto program = gl::create_program();

    // Vertex shader
    auto vertex_shader = gl::create_shader(GL_VERTEX_SHADER);
    gl::shader_source(vertex_shader, resources::vertex_shader_vsh.data());
    gl::compile_shader(vertex_shader);
    gl::attach_shader(program, vertex_shader);

    // Fragment shader
    auto fragment_shader = gl::create_shader(GL_FRAGMENT_SHADER);
    gl::shader_source(fragment_shader, resources::fragment_shader_fsh.data());
    gl::compile_shader(fragment_shader);
    gl::attach_shader(program, fragment_shader);

    gl::link_program(program);

    const auto projection_uniform = gl::get_uniform_location(program, "projection_matrix");

    // Set clear color to magenta
    gl::clear_color(1.0f, 0.0f, 1.0f, 1.0f);

    auto vertex_array_object = gl::generate_vertex_array_object();
    auto vertex_buffer_object = gl::vertex_buffer_object_t::create_buffer_object(vertex_positions, program, "vertex_position");
    auto index_buffer_object = gl::index_buffer_object_t::create_buffer_object(vertex_indices);
    auto texture_coordinate_buffer_object = gl::texture_coordinate_buffer_object_t::create_buffer_object(vertex_uvs, program, "vertex_uv");
    auto model_matrix_buffer_object = gl::model_matrix_buffer_object_t::create_buffer_object(program, "model_matrix");
    auto model_color_buffer_object = gl::model_color_buffer_object_t::create_buffer_object(program, "model_color");
    auto vertex_width_buffer_object = gl::vertex_width_buffer_object_t::create_buffer_object(program, "vertex_width");

    return {
        std::move(program),
        std::move(vertex_shader),
        std::move(fragment_shader),
        projection_uniform,
        std::move(vertex_array_object),
        std::move(vertex_buffer_object),
        std::move(index_buffer_object),
        std::move(texture_coordinate_buffer_object),
        std::move(model_matrix_buffer_object),
        std::move(model_color_buffer_object),
        std::move(vertex_width_buffer_object)
    };
}

void debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *) {
    std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **"sv : ""sv) << std::hex << " type = 0x" << type << ", severity = 0x" << severity << ", message = " << message << std::endl << std::dec;
}

void render(const shader_stuff& stuff, const glm::mat4& projection_matrix, float delta_time, const std::vector<line>& lines) {
    std::vector<glm::mat4> model_matrixes;
    for (auto& line : lines) {
        model_matrixes.push_back(line.transform_matrix());
    }

    std::vector<glm::vec3> model_colors;
    for (auto& line : lines) {
        model_colors.push_back(line.color());
    }

    std::vector<glm::vec2> vertex_widths;
    for (auto& line : lines) {
        vertex_widths.push_back({line.start_width(), line.end_width()});
    }

    gl::clear(GL_COLOR_BUFFER_BIT);

    gl::use_program(stuff.program);


    stuff.vertex_buffer_object.bind();
    stuff.vertex_buffer_object.upload();

    stuff.texture_coordinate_buffer_object.bind();
    stuff.texture_coordinate_buffer_object.upload();

    stuff.model_matrix_buffer_object.set_data(model_matrixes);
    stuff.model_matrix_buffer_object.upload();

    stuff.model_color_buffer_object.set_data(model_colors);
    stuff.model_color_buffer_object.upload();

    stuff.vertex_width_buffer_object.set_data(vertex_widths);
    stuff.vertex_width_buffer_object.upload();

    gl::uniform_matrix(stuff.projection_uniform, projection_matrix);

    stuff.index_buffer_object.bind();
    //glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, vertex_indices.size(), lines.size());

    gl::unbind_program();
}

