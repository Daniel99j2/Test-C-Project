#include "GenericUtil.h"

#include <iostream>
#include <chrono>
#include <random>

#include "GameConstants.h"
using namespace std;

int GenericUtil::randomInt(int min, int max) {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    minstd_rand0 generator (seed);
    return min + generator() % (max - min + 1);
}

float GenericUtil::randomFloat(float min, float max) {
    return randomFloat(min, max, 8);
}

float GenericUtil::randomFloat(float min, float max, int decimals) {
    float divider = 1;
    for (int i = 0; i < decimals; ++i) {
        divider *= 10;
    }
    return randomInt(min*divider, max*divider)/divider;
}

glm::vec3 GenericUtil::moveVec3(glm::vec3 vec, double acceleration, float pitch, float yaw) {
    double yawRad = glm::radians(yaw);
    double pitchRad = glm::radians(pitch);

    double x = -glm::sin(yawRad) * glm::cos(pitchRad);
    double y = -glm::sin(pitchRad);
    double z = glm::cos(yawRad) * glm::cos(pitchRad);

    glm::vec3 accelerationVector(x, y, z);
    accelerationVector = glm::normalize(accelerationVector);
    return vec = vec + accelerationVector * glm::vec3(acceleration);
}

//guess what monitor is used
GLFWmonitor* GenericUtil::getCurrentMonitor(GLFWwindow* window) {
    int nmonitors;
    GLFWmonitor** monitors = glfwGetMonitors(&nmonitors);
    if (!monitors) return nullptr;

    int wx, wy;
    glfwGetWindowPos(window, &wx, &wy);

    GLFWmonitor* bestMonitor = nullptr;
    int bestOverlap = 0;

    for (int i = 0; i < nmonitors; i++) {
        GLFWmonitor* monitor = monitors[i];

        int mx, my;
        glfwGetMonitorPos(monitor, &mx, &my);

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (!mode) continue;

        int mw = mode->width;
        int mh = mode->height;

        int overlapX = std::max(0, std::min(wx + GameConstants::window_width, mx + mw) - std::max(wx, mx));
        int overlapY = std::max(0, std::min(wy + GameConstants::window_height, my + mh) - std::max(wy, my));
        int overlapArea = overlapX * overlapY;

        if (overlapArea > bestOverlap) {
            bestOverlap = overlapArea;
            bestMonitor = monitor;
        }
    }

    return bestMonitor;
}
