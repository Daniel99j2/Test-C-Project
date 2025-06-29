//
// Created by dj on 22/06/2025.
//

#include "World.h"

#include "src/c/objects/GameObject.h"

void World::drawWorld(float deltaTime) const {
    for (auto object : gameObjects) {
        object->draw(deltaTime);
    }
}

void World::tick() const {
    for (auto object : gameObjects) {
        object->baseTick();
    }
}

void World::addObject(std::shared_ptr<GameObject> object) {
    object.get()->id = currentId;
    gameObjects.push_back(object);
    currentId++;
}