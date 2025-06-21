#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"
#include "GenericUtil.h"

#include <iostream>
#include <chrono>
#include <random>
#include "GenericUtil.h"

#include "libs/glm/vec3.hpp"
#include "libs/glm/detail/func_geometric.inl"
#include "libs/glm/detail/func_trigonometric.inl"
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