#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "wrappers/sdl.hpp"
#include "wrappers/glew.hpp"
#include "wrappers/opengl.hpp"

#include "generated/resources.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

struct shader_stuff {
    gl::program_t program;
    gl::shader_t vertex_shader;
    gl::shader_t fragment_shader;
    GLint vertex_position_attribute;
    GLint vertex_uv_attribute;
    GLint projection_uniform;
    GLint model_uniform;
    gl::vertex_array_object_t vertex_array_object;
    gl::vertex_buffer_object_t vertex_buffer_object;
    gl::index_buffer_object_t index_buffer_object;
    gl::texture_coordinate_buffer_object_t texture_coordinate_buffer_object;
};
shader_stuff init_gl();
void render(const shader_stuff& stuff, const glm::mat4& projection);

void GLAPIENTRY debug_message_callback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void*);

int main() {
    sdl::init_sub_system(SDL_INIT_TIMER);
    sdl::init_sub_system(SDL_INIT_VIDEO);
    sdl::init_sub_system(SDL_INIT_EVENTS);

    {
        auto quit = false;

        sdl::gl_set_attribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        sdl::gl_set_attribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        sdl::gl_set_attribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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
                        std::cout << "Quitting..." << "\n";
                        quit = true;
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                        if (pool_event_result.event.button.button == SDL_BUTTON_LEFT) {
                            auto x = pool_event_result.event.button.x;
                            auto y = pool_event_result.event.button.y;
                            std::cout << "button pressed at (" << x << ", " << y << ")\n";
                        }
                        break;

                    case SDL_WINDOWEVENT:
                        if (pool_event_result.event.window.event == SDL_WINDOWEVENT_RESIZED) {
                            auto x = pool_event_result.event.window.data1;
                            auto y = pool_event_result.event.window.data2;

                            std::cout << "window size: " << x << ", " << y << "\n";

                            projection_matrix = glm::ortho(0.0f, static_cast<float>(x), 0.0f, static_cast<float>(y), -100.0f, 100.0f);
                            glViewport(0, 0, x, y);
                        }
                    break;
                }
            }

            render(stuff, projection_matrix);

            sdl::gl_swap_window(window);
        }
    }

    sdl::quit();

    return EXIT_SUCCESS;
}

 shader_stuff init_gl() {
    gl::enable(GL_DEBUG_OUTPUT);
    gl::debug_message_callback(debug_message_callback, nullptr);

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

    auto vertex_position_attribute = gl::get_attribute_location(program, "vertex_position");
    auto vertex_uv_attribute = gl::get_attribute_location(program, "vertex_uv");
    auto projection_uniform = gl::get_uniform_location(program, "projection");
    auto model_uniform = gl::get_uniform_location(program, "model");

    // Set clear color to magenta
    gl::clear_color(1.0f, 0.0f, 1.0f, 1.0f);

    //VBO data
    std::vector<GLfloat> vertex_data =
    {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f
    };

    std::vector<GLfloat> uv = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    };

    //IBO data
    std::vector<GLuint> index_data = { 0, 1, 2, 3 };

    auto vertex_array_object = gl::generate_vertex_array_object();
    auto vertex_buffer_object = gl::generate_vertex_buffer_object(vertex_data);
    auto index_buffer_object = gl::generate_index_buffer_object(index_data);
    auto texture_coordinate_buffer_object = gl::generate_texture_coordinate_buffer_object(uv);

    return { std::move(program), std::move(vertex_shader), std::move(fragment_shader), vertex_position_attribute, vertex_uv_attribute, projection_uniform, model_uniform, std::move(vertex_array_object), std::move(vertex_buffer_object), std::move(index_buffer_object), std::move(texture_coordinate_buffer_object) };
}

void debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *) {
    std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **"sv : ""sv) << std::hex << " type = 0x" << type << ", severity = 0x" << severity << ", message = " << message << std::endl << std::dec;
}

void render(const shader_stuff& stuff, const glm::mat4& projection_matrix) {
    static float rotation = 0.0f;

    rotation = std::fmod(rotation + 0.05f, 360.0f);
    auto model_matrix = glm::identity<glm::mat4>();
    auto rotation_matrix = glm::rotate(model_matrix, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    auto scale_matrix = glm::scale(model_matrix, glm::vec3(200.0f, 200.0f, 1.0f));
    auto translation_matrix = glm::translate(model_matrix, glm::vec3(200.0f, 200.0f, 0.0f));

    model_matrix = translation_matrix * rotation_matrix * scale_matrix;


    gl::clear(GL_COLOR_BUFFER_BIT);

    gl::use_program(stuff.program);


    gl::bind_vertex_buffer_object(stuff.vertex_buffer_object);
    gl::enable_vertex_attribute_array(stuff.vertex_position_attribute);
    gl::vertex_attrib_pointer(stuff.vertex_position_attribute);

    gl::bind_texture_coordinate_buffer_object(stuff.texture_coordinate_buffer_object);
    gl::enable_vertex_attribute_array(stuff.vertex_uv_attribute);
    gl::vertex_attrib_pointer(stuff.vertex_uv_attribute);

    glUniformMatrix4fv(stuff.projection_uniform, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix4fv(stuff.model_uniform, 1, GL_FALSE, glm::value_ptr(model_matrix));

    gl::bind_index_buffer_object(stuff.index_buffer_object);
    gl::draw_elements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT);

    gl::disable_vertex_attribute_array(stuff.vertex_position_attribute);
    gl::disable_vertex_attribute_array(stuff.vertex_uv_attribute);

    gl::unbind_program();
}

