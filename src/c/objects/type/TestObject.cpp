//
// Created by dj on 28/06/2025.
//

#include "TestObject.h"
#include "src/c/util/ModelUtil.h"

TestObject::TestObject(const glm::vec3& vec)
    : LivingObject(vec) {
    maxHealth = 100;
    health = 100;
    damage = 10;
    speed = 5;
    type = "player";
    model = ModelUtil::getModel("hello");
}

void TestObject::tick() {

}