//
// Created by dj on 22/06/2025.
//

#pragma once
#include "GameObject.h"

class LivingObject: public GameObject {
public:
    int health = 0;
    int maxHealth = 0;
    int damage = 0;
    int speed = 0;

    explicit LivingObject(const glm::vec3& vec) : GameObject(vec) {

    };

    virtual ~LivingObject() = default;
};