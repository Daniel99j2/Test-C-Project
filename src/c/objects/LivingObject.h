//
// Created by dj on 22/06/2025.
//

#pragma once
#include "GameObject.h"

class LivingObject: public GameObject {
public:
    float health = 0;
    float maxHealth = 0;
    float damage = 0;
    float speed = 0;

    explicit LivingObject(const glm::vec3& vec) : GameObject(vec) {

    };

    virtual ~LivingObject() = default;
};