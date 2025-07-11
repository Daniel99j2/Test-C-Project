#pragma once
#include <memory>
#include <vector>
#include <unordered_map>

#include "../objects/GameObject.h"

class World {
public:
    void drawWorld(float deltaTime) const;

    void tick(float deltaTime);

    void addObject(std::shared_ptr<GameObject> object);

    [[nodiscard]] std::vector<std::shared_ptr<GameObject>> getObjects() const {
        return gameObjects;
    }

private:
    std::vector<std::shared_ptr<GameObject>> gameObjects;
    int currentId = 0;

    void simulatePhysics(float deltaTime);
    void handleCollisions() const;
};