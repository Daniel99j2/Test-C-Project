//
// Created by dj on 28/06/2025.
//
#pragma once

#include "src/c/objects/LivingObject.h"

class TestObject : public LivingObject {
public:
    explicit TestObject(const glm::vec3& vec);

    void tick() override;
};
