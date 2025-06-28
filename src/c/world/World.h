//
// Created by dj on 22/06/2025.
//

#pragma once
#include <memory>
#include <vector>

#include "src/c/objects/GameObject.h"

class World {
public:
    void drawWorld() const;

    void tick() const;

    void addObject(std::shared_ptr<GameObject> object);

private:
    //we do a shared_ptr so it persists
    std::vector<std::shared_ptr<GameObject>> gameObjects;
    int currentId = 0;
};