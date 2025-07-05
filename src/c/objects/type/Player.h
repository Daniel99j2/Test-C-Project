//
// Created by dj on 22/06/2025.
//

#pragma once
#include "../LivingObject.h"

class Player : public LivingObject {
public:
    explicit Player(const glm::vec3& vec);

    void tick() override;

    void move(bool w, bool a, bool s, bool d);
};