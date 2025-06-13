cmake_minimum_required(VERSION 3.31)
project(Main)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/glew/build/cmake)
add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)

find_package(OpenGL REQUIRED)

add_executable(
        Main src/c/main.cpp
        src/c/util/RenderUtil.cpp
        src/c/util/RenderUtil.h
        src/c/util/GenericUtil.cpp
        src/c/util/GenericUtil.h
        libs/stb_image.h
        src/c/util/Shader.h
)

file(COPY src/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/src)

target_link_libraries(Main PRIVATE glew)
target_link_libraries(Main PRIVATE glfw)
target_link_libraries(Main PRIVATE glm)
