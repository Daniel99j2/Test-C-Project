//
// Created by dj on 22/06/2025.
//

#include "World.h"

#include "src/c/objects/GameObject.h"
#include "src/c/util/GameConstants.h"

void World::drawWorld(float deltaTime) const {
    for (auto object : gameObjects) {
        object->draw(deltaTime);
    }
}

void World::drawDepth(Shader shader) const {
    for (const auto& object : gameObjects) {
        object->drawDepth(shader);
    }
}

void World::tick() const {
    for (const auto& object : gameObjects) {
        object->baseTick();
    }
}

void World::addObject(std::shared_ptr<GameObject> object) {
    object.get()->id = currentId;
    gameObjects.push_back(object);
    GameConstants::physicsEngine.addObject(object);
    currentId++;
}