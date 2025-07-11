//
// Created by dj on 7/07/2025.
//

//This speeds up loading by having everything already compiled seperately

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM

#include <GL/glew.h>
#include <iostream>
#include <chrono>
#include <random>
#include <windows.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <thread>

#include "../../../libs/json.hpp"
#include "../../../libs/imgui/imgui.h"
#include "../../../libs/imgui/backends/imgui_impl_glfw.h"
#include "../../libs/imgui/backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>

#include "../../../libs/packer/rectpack2D/best_bin_finder.h"
#include "../../../libs/packer/rectpack2D/finders_interface.h"
#include "../../../libs/packer/rectpack2D/rect_structs.h"