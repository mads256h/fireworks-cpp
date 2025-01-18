cmake_minimum_required(VERSION 3.21)
project(fireworks_cpp)

set(CMAKE_CXX_STANDARD 26)

# Generate clang compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else ()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic -Wnarrowing)
    add_compile_options(-Wno-ignored-attributes)
endif ()

include(cmake/sanitizers.cmake)
include(cmake/include-what-you-use.cmake)

add_executable(resource_embedder tools/resource_embedder.cpp)

set(RESOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/embed")
file(GLOB RESOURCE_FILES "${RESOURCE_DIRECTORY}/*")

set(GENERATED_RESOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(GENERATED_RESOURCE_HPP_FILE ${GENERATED_RESOURCE_DIRECTORY}/resources.hpp)
set(GENERATED_RESOURCE_CPP_FILE ${GENERATED_RESOURCE_DIRECTORY}/resources.cpp)

file(MAKE_DIRECTORY "${GENERATED_RESOURCE_DIRECTORY}")

add_custom_command(
    OUTPUT ${GENERATED_RESOURCE_HPP_FILE} ${GENERATED_RESOURCE_CPP_FILE}
    COMMAND resource_embedder
    ARGS ${GENERATED_RESOURCE_HPP_FILE} ${GENERATED_RESOURCE_CPP_FILE} ${RESOURCE_FILES}
    DEPENDS ${RESOURCE_FILES}
    COMMENT "Generating ${GENERATED_RESOURCE_FILE}."
    VERBATIM
)

add_custom_target(embed_resources ALL DEPENDS ${GENERATED_RESOURCE_HPP_FILE} ${GENERATED_RESOURCE_CPP_FILE})

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

if (NOT DEFINED IMGUI_LIBRARIES AND NOT DEFINED IMGUI_INCLUDE_DIRS)
       find_package(imgui REQUIRED)
       set(IMGUI_LIBRARIES imgui::imgui)
endif ()



add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

include_directories(fireworks_cpp ${GENERATED_RESOURCE_DIRECTORY} ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${IMGUI_INCLUDE_DIRS})

add_executable(fireworks_cpp src/main.cpp
        src/primitives.cpp
        src/wrappers/opengl/shader.cpp
        src/wrappers/opengl.cpp
        src/wrappers/sdl.cpp
        src/wrappers/glew.cpp
        ${GENERATED_RESOURCE_CPP_FILE})
target_link_libraries(fireworks_cpp ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLM_LIBRARIES} ${IMGUI_LIBRARIES})
add_dependencies(fireworks_cpp embed_resources)

try_enable_include_what_you_use(fireworks_cpp mapping_file.imp)
