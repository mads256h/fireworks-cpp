cmake_minimum_required(VERSION 3.30)
project(fireworks_cpp)

function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output} "#include <string_view>\nusing namespace std::string_view_literals;\nnamespace resources {\n")
    # Collect input files
    file(GLOB bins ${dir}/*)
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "\"" "\\\\\"," filedata "${filedata}")
        string(REGEX REPLACE "\r" "\\\\r" filedata "${filedata}")
        string(REGEX REPLACE "\n" "\\\\n" filedata "${filedata}")
        # Append data to output file
        file(APPEND ${output} "constexpr auto ${filename} = \"${filedata}\"sv;\n")
    endforeach()
    file(APPEND ${output} "}")
endfunction()

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)


include_directories(fireworks_cpp ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 26)

create_resources(embed "generated/resources.hpp")

add_executable(fireworks_cpp main.cpp
        primitives.cpp)
target_link_libraries(fireworks_cpp ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLM_LIBRARIES})

