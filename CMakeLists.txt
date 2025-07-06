cmake_minimum_required(VERSION 3.31)
project(Main)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/glew/build/cmake)
add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)
# Note that packer is modified!
add_subdirectory(libs/packer)

find_package(OpenGL REQUIRED)

add_executable(
        Main src/c/main.cpp
        src/c/util/RenderUtil.cpp
        src/c/util/RenderUtil.h
        src/c/util/GenericUtil.cpp
        src/c/util/GenericUtil.h
        libs/stb_image.h
        libs/stb_image_write.h
        libs/json.hpp
        src/c/util/ModelUtil.cpp
        src/c/util/ModelUtil.h
        src/c/util/Mesh.h
        src/c/objects/GameObject.cpp
        src/c/objects/GameObject.h
        src/c/objects/LivingObject.cpp
        src/c/objects/LivingObject.h
        src/c/objects/type/Player.cpp
        src/c/objects/type/Player.h
        src/c/world/World.cpp
        src/c/world/World.h
        src/c/util/GameConstants.h
        src/c/util/GameConstants.cpp
        src/c/objects/type/SimpleObject.cpp
        src/c/objects/type/SimpleObject.h
        src/c/util/Animation.h
        src/c/util/AnimatorInstance.cpp
        src/c/util/AnimatorInstance.h
        src/c/util/Keybind.h
        src/c/util/Keybinds.h
        src/c/util/Keybind.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp
        libs/imgui/imgui.cpp
        libs/imgui/imgui_demo.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_tables.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp
        src/c/util/Profiler.h
        src/c/util/Logger.h
)

include_directories(
        libs/imgui
        libs/imgui/backends
)

file(COPY src/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/src)

# This makes it so the .dlls and libraries are in it
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# This makes it so the src folder is actually copied
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resources
        ${CMAKE_CURRENT_BINARY_DIR}/src/resources
        COMMENT "Copying sources"
)

target_sources(Main PRIVATE
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp
        libs/imgui/imgui.cpp
        libs/imgui/imgui_demo.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_tables.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(Main PRIVATE glew)
target_link_libraries(Main PRIVATE glfw)
target_link_libraries(Main PRIVATE glm)
target_link_libraries(Main PRIVATE rectpack2D)