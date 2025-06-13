//
// Created by dj on 13/06/2025.
//

#ifndef GENERICUTIL_H
#define GENERICUTIL_H
#include "libs/glm/vec3.hpp"


class GenericUtil {
public:
    static int randomInt(int i, int i1);
    static glm::vec3 moveVec3(glm::vec3 vec, double acceleration, float pitch, float yaw);
};



#endif //GENERICUTIL_H
