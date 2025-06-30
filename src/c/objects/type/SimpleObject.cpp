//
// Created by dj on 28/06/2025.
//

#include "SimpleObject.h"
#include "src/c/util/ModelUtil.h"

SimpleObject::SimpleObject(const glm::vec3& vec)
    : LivingObject(vec) {
    type = "simple";
    model = ModelUtil::getModel("hello");
}

void SimpleObject::tick() {

}