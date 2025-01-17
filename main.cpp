#include <SDL2/SDL.h>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "wrappers/sdl.hpp"
#include "wrappers/glew.hpp"
#include "wrappers/opengl.hpp"

#include "primitives.hpp"

#include "resources.hpp"

#include <cstdlib>
#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <format>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std::string_view_literals;

constexpr std::array<glm::vec2, 4> vertex_positions = {glm::vec2{-0.5f, -0.5f},
                                                       {0.5f, -0.5f},
                                                       {0.5f, 0.5f},
                                                       {-0.5f, 0.5f}};

constexpr std::array<glm::vec2, 4> vertex_uvs = {glm::vec2{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

constexpr std::array<GLuint, 4> vertex_indices = {0, 1, 2, 3};

struct star_shader_stuff_t {
    gl::program_t program;
    gl::vertex_shader_t vertex_shader;
    gl::fragment_shader_t fragment_shader;
    gl::vertex_array_object_t vertex_array_object;
    gl::vertex_buffer_object_t vertex_buffer_object;
    gl::index_buffer_object_t index_buffer_object;
    gl::uniform_location_t projection_uniform;
    gl::uniform_location_t background_color_uniform;
    gl::uniform_location_t star_density_uniform;
};

struct line_shader_stuff_t {
    gl::program_t program;
    gl::vertex_shader_t vertex_shader;
    gl::fragment_shader_t fragment_shader;
    gl::uniform_location_t projection_uniform;
    gl::vertex_array_object_t vertex_array_object;
    gl::vertex_buffer_object_t vertex_buffer_object;
    gl::index_buffer_object_t index_buffer_object;
    gl::texture_coordinate_buffer_object_t texture_coordinate_buffer_object;
    gl::model_matrix_buffer_object_t model_matrix_buffer_object;
    gl::model_color_buffer_object_t model_color_buffer_object;
    gl::vertex_width_buffer_object_t vertex_width_buffer_object;
};

struct shader_stuff_t {
    star_shader_stuff_t star_shader_stuff;
    line_shader_stuff_t line_shader_stuff;
};

struct window_state_t {
    bool m_show_fps = true;
    glm::vec3 m_background_color = glm::vec3(0.0f);
    float m_star_density = 0.98f;
};

shader_stuff_t init_gl();

void render(const shader_stuff_t& stuff,
            const glm::mat4& projection_matrix,
            const window_state_t& window_state,
            const std::vector<line>& lines,
            const glm::vec2& window_size);

void GLAPIENTRY debug_message_callback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void*);

window_state_t& render_debug_menu(bool render_imgui) {
    static window_state_t state;

    if (render_imgui) {
        ImGui::Begin("Debug menu");
        ImGui::Checkbox("Show FPS", &state.m_show_fps);
        ImGui::ColorPicker3("Background Color", glm::value_ptr(state.m_background_color));
        ImGui::DragFloat("Star Density", &state.m_star_density, 0.001f, 0.0f, 1.0f);
        ImGui::End();
    }

    return state;
}

extern "C" int main(int, char**) {
    sdl::init_sub_system(SDL_INIT_TIMER);
    sdl::init_sub_system(SDL_INIT_VIDEO);
    sdl::init_sub_system(SDL_INIT_EVENTS); {
        std::vector<line> lines;
        auto quit = false;
        bool got_first_point = false;
        glm::vec2 first_point = glm::vec2{0.0f};

        auto last_fps_update = 1.0f;
        auto frames_this_update = 0;
        std::string fps;

        auto render_imgui = true;

        const auto performance_frequency = static_cast<double>(sdl::get_performance_frequency());
        auto delta_time = 0.016f; // 1 frame at 60 fps initially.
        auto last_time = static_cast<float>(static_cast<double>(sdl::get_performance_counter()) /
                                            performance_frequency);

        sdl::gl_set_attribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        sdl::gl_set_attribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        sdl::gl_set_attribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        sdl::gl_set_attribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        sdl::gl_set_attribute(SDL_GL_MULTISAMPLESAMPLES, 8);

        // Needed by IMGUI
        sdl::gl_set_attribute(SDL_GL_DOUBLEBUFFER, 1);
        sdl::gl_set_attribute(SDL_GL_DEPTH_SIZE, 24);
        sdl::gl_set_attribute(SDL_GL_STENCIL_SIZE, 8);

        auto window = sdl::create_window("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                                         SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                         SDL_WINDOW_ALLOW_HIGHDPI);

        auto gl_context = sdl::gl_create_context(window);
        glew::init();

        // Use vsync
        sdl::gl_try_use_vsync();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(window.get(), gl_context.value());
        ImGui_ImplOpenGL3_Init("#version 130");

        auto stuff = init_gl();

        auto projection_matrix = glm::identity<glm::mat4>();

        glm::vec2 window_size{1280, 720};

        while (!quit) {
            for (auto pool_event_result = sdl::pool_event(); pool_event_result.pending_event;
                 pool_event_result = sdl::pool_event()) {
                ImGui_ImplSDL2_ProcessEvent(&pool_event_result.event);

                switch (pool_event_result.event.type) {
                    case SDL_QUIT:
                        std::cout << "Quitting..." << std::endl;
                        quit = true;
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                        if (pool_event_result.event.button.button == SDL_BUTTON_LEFT && !ImGui::IsWindowHovered(
                                    ImGuiHoveredFlags_AnyWindow)) {
                            auto x = pool_event_result.event.button.x;
                            auto y = pool_event_result.event.button.y;
                            if (!got_first_point) {
                                first_point = glm::vec2{x, y};
                                got_first_point = true;
                            } else {
                                glm::vec2 end_position{x, y};
                                lines.emplace_back(first_point, end_position, glm::vec3(1.0f), 50.0f, 20.0f);
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

                            projection_matrix = glm::ortho(0.0f, static_cast<float>(x), static_cast<float>(y), 0.0f,
                                                           -1.0f, 1.0f);
                            glViewport(0, 0, x, y);
                            window_size = {x, y};
                        }
                        break;

                    case SDL_KEYDOWN:
                        if (pool_event_result.event.key.keysym.scancode == SDL_SCANCODE_F9) {
                            render_imgui = !render_imgui;
                        }
                        break;
                }
            }

            if (render_imgui) {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplSDL2_NewFrame();
                ImGui::NewFrame();
            }

            if (last_fps_update >= 1.0f) {
                auto frames = static_cast<float>(frames_this_update) / last_fps_update;
                fps = std::format("{:.0f} fps", frames);
                last_fps_update = 0.0f;
                frames_this_update = 0;
            }

            last_fps_update += delta_time;
            frames_this_update++;

            auto window_state = render_debug_menu(render_imgui);

            if (window_state.m_show_fps && render_imgui) {
                ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(0.0f, 0.0f),
                                                        ImColor(1.0f, 1.0f, 1.0f), fps.c_str(), nullptr, 0.0f, nullptr);
            }

            //ImGui::ShowDemoWindow();

            render(stuff, projection_matrix, window_state, lines, window_size);

            if (render_imgui) {
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            auto now = static_cast<float>(static_cast<double>(sdl::get_performance_counter()) / performance_frequency);
            delta_time = now - last_time;
            last_time = now;

            sdl::gl_swap_window(window);
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }


    sdl::quit();

    return EXIT_SUCCESS;
}

star_shader_stuff_t create_star_shader() {
    auto program = gl::create_program();

    auto vertex_shader = gl::vertex_shader_t::create_shader(program, resources::star_vertex_shader_vsh);

    auto fragment_shader = gl::fragment_shader_t::create_shader(program, resources::star_fragment_shader_fsh);

    gl::link_program(program);

    auto projection_uniform = gl::get_uniform_location(program, "projection_matrix");
    auto background_color_uniform = gl::get_uniform_location(program, "background_color");
    auto star_density_uniform = gl::get_uniform_location(program, "star_density");

    auto vertex_array_object = gl::generate_vertex_array_object();
    auto vertex_buffer_object = gl::vertex_buffer_object_t::create_buffer_object(program, "vertex_position");
    auto index_buffer_object = gl::index_buffer_object_t::create_buffer_object(vertex_indices);

    return {std::move(program),
            std::move(vertex_shader),
            std::move(fragment_shader),
            std::move(vertex_array_object),
            std::move(vertex_buffer_object),
            std::move(index_buffer_object),
            projection_uniform,
            background_color_uniform,
            star_density_uniform};
}

line_shader_stuff_t create_line_shader() {
    auto program = gl::create_program();

    // Vertex shader
    auto vertex_shader = gl::vertex_shader_t::create_shader(program, resources::vertex_shader_vsh);

    // Fragment shader
    auto fragment_shader = gl::fragment_shader_t::create_shader(program, resources::fragment_shader_fsh);

    gl::link_program(program);

    const auto projection_uniform = gl::get_uniform_location(program, "projection_matrix");


    auto vertex_array_object = gl::generate_vertex_array_object();
    auto vertex_buffer_object = gl::vertex_buffer_object_t::create_buffer_object(
            vertex_positions, program, "vertex_position");
    auto index_buffer_object = gl::index_buffer_object_t::create_buffer_object(vertex_indices);
    auto texture_coordinate_buffer_object = gl::texture_coordinate_buffer_object_t::create_buffer_object(
            vertex_uvs, program, "vertex_uv");
    auto model_matrix_buffer_object = gl::model_matrix_buffer_object_t::create_buffer_object(program, "model_matrix");
    auto model_color_buffer_object = gl::model_color_buffer_object_t::create_buffer_object(program, "model_color");
    auto vertex_width_buffer_object = gl::vertex_width_buffer_object_t::create_buffer_object(program, "vertex_width");

    return {std::move(program),
            std::move(vertex_shader),
            std::move(fragment_shader),
            projection_uniform,
            std::move(vertex_array_object),
            std::move(vertex_buffer_object),
            std::move(index_buffer_object),
            std::move(texture_coordinate_buffer_object),
            std::move(model_matrix_buffer_object),
            std::move(model_color_buffer_object),
            std::move(vertex_width_buffer_object)};
}

shader_stuff_t init_gl() {
#ifndef NDEBUG
    gl::enable(GL_DEBUG_OUTPUT);
    gl::debug_message_callback(debug_message_callback, nullptr);
#endif

    gl::enable(GL_BLEND);
    gl::enable(GL_MULTISAMPLE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set clear color to magenta
    gl::clear_color(1.0f, 0.0f, 1.0f, 1.0f);

    auto star_shader_stuff = create_star_shader();
    auto line_shader_stuff = create_line_shader();


    return {std::move(star_shader_stuff), std::move(line_shader_stuff)};
}

void debug_message_callback(GLenum source,
                            GLenum type,
                            GLuint,
                            GLenum severity,
                            GLsizei,
                            const GLchar* message,
                            const void*) {
    const char* _source;
    const char* _type;
    const char* _severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
            _source = "OTHER";
            break;

        default:
            _source = "UNKNOWN";
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UNDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;

        default:
            _type = "UNKNOWN";
            break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION";
            break;

        default:
            _severity = "UNKNOWN";
            break;
    }

    std::cerr << "GL CALLBACK: source = " << _source << ", type = " << _type << ", severity = " << _severity <<
            ", message = " << message << std::endl;
}


template<typename T, typename F>
void set_uniform_if_changed(T& old_value,
                            const T& new_value,
                            gl::uniform_location_t uniform_location,
                            F uniform_setter) {
    if (old_value == new_value) {
        return;
    }

    uniform_setter(uniform_location, new_value);
    old_value = new_value;
}

void render_stars(const star_shader_stuff_t& stuff,
                  const window_state_t& window_state,
                  const glm::mat4& projection_matrix,
                  glm::vec2 window_size) {
    static glm::mat4 old_projection_matrix = glm::identity<glm::mat4>();
    static glm::vec2 old_window_size = {0.0f, 0.0f};
    static glm::vec3 old_background_color = {0.0f, 0.0f, 0.0f};
    static float old_star_density = 0.0f;

    gl::use_program(stuff.program);


    set_uniform_if_changed(old_projection_matrix, projection_matrix, stuff.projection_uniform, gl::uniform_matrix);
    set_uniform_if_changed(old_background_color, window_state.m_background_color, stuff.background_color_uniform,
                           gl::uniform_vec3);
    set_uniform_if_changed(old_star_density, window_state.m_star_density, stuff.star_density_uniform,
                           gl::uniform_float);

    if (old_window_size != window_size) {
        const std::array<glm::vec2, 4> vertex_positions = {glm::vec2{0.0f, 0.0f},
                                                           {window_size.x, 0.0f},
                                                           {window_size.x, window_size.y},
                                                           {0.0f, window_size.y}};

        stuff.vertex_buffer_object.set_data(vertex_positions);
        old_window_size = window_size;
    } else {
        stuff.vertex_buffer_object.bind();
    }
    stuff.vertex_buffer_object.upload();

    stuff.index_buffer_object.bind();
    gl::draw_arrays(GL_TRIANGLE_FAN, 1, 4);

    gl::unbind_program();
}

void render_lines(const line_shader_stuff_t& stuff,
                  const glm::mat4& projection_matrix,
                  const std::vector<line>& lines) {
    static glm::mat4 old_projection_matrix = glm::identity<glm::mat4>();

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
        vertex_widths.emplace_back(line.start_width(), line.end_width());
    }


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

    set_uniform_if_changed(old_projection_matrix, projection_matrix, stuff.projection_uniform, gl::uniform_matrix);

    stuff.index_buffer_object.bind();
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, vertex_indices.size(), lines.size());

    gl::unbind_program();
}

void render(const shader_stuff_t& stuff,
            const glm::mat4& projection_matrix,
            const window_state_t& window_state,
            const std::vector<line>& lines,
            const glm::vec2& window_size) {
    gl::clear(GL_COLOR_BUFFER_BIT);
    gl::enable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    render_stars(stuff.star_shader_stuff, window_state, projection_matrix, window_size);
    render_lines(stuff.line_shader_stuff, projection_matrix, lines);
}


