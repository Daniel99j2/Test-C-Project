//
// Created by dj on 28/06/2025.
//
#pragma once

#include "../LivingObject.h"

class SimpleObject : public LivingObject {
public:
    explicit SimpleObject(const glm::vec3& vec);

    void tick() override;
};
