//
// Created by dj on 13/06/2025.
//

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>


class GenericUtil {
public:
    static int randomInt(int i, int i1);

    static float randomFloat(float min, float max, int i);

    static float randomFloat(float min, float max);

    static glm::vec3 moveVec3(glm::vec3 vec, double acceleration, float pitch, float yaw);

    static GLFWmonitor *getCurrentMonitor(GLFWwindow *window);
};