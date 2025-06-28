//
// Created by dj on 22/06/2025.
//


#pragma once
#include <vector>
#include <string>
#include <memory>

#include "libs/glm/vec3.hpp"

enum class ShapeType {
    Rectangle,
    Sphere,
    Cylinder
};

struct Collision {
    std::shared_ptr<class PhysicsObject> other;
    glm::vec3 point;
};

class PhysicsObject {
public:
    ShapeType shape;
    glm::vec3 position;
    glm::vec3 velocity{};
    glm::vec3 size; // radius for sphere, size for rectangle/cylinder
    float mass;
    float gravity;
    bool isStatic = false;

    std::vector<Collision> collisions;

    PhysicsObject(ShapeType shape, glm::vec3 pos, glm::vec3 size, float mass, float gravity);

    PhysicsObject();

    void applySlowdown(float drag);
    void update(float dt);
};

class PhysicsEngine {
public:
    std::vector<std::shared_ptr<PhysicsObject>> objects;

    void addObject(std::shared_ptr<PhysicsObject> obj);
    void simulate(float dt);
private:
    void handleCollisions();
};