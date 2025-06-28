//
// Created by dj on 22/06/2025.
//

#include "PhysicsEngine.h"

#include "libs/glm/geometric.hpp"
#include "libs/glm/vec3.hpp"
#include "libs/glm/glm.hpp"
#include <unordered_map>
#include <cmath>

constexpr float GRID_SIZE = 5.0f;

struct GridCoord {
    int x, y, z;
    bool operator==(const GridCoord &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {
    template <>
    struct hash<GridCoord> {
        std::size_t operator()(const GridCoord &g) const {
            return ((hash<int>()(g.x) ^ (hash<int>()(g.y) << 1)) >> 1) ^ (hash<int>()(g.z) << 1);
        }
    };
}

GridCoord toGrid(const glm::vec3 &pos) {
    return {
        static_cast<int>(std::floor(pos.x / GRID_SIZE)),
        static_cast<int>(std::floor(pos.y / GRID_SIZE)),
        static_cast<int>(std::floor(pos.z / GRID_SIZE))
    };
}

PhysicsObject::PhysicsObject(ShapeType shape, glm::vec3 pos, glm::vec3 size, float mass, float gravity)
    : shape(shape), position(pos), size(size), mass(mass), gravity(gravity) {}

PhysicsObject::PhysicsObject() {

}


void PhysicsObject::applySlowdown(float drag) {
    velocity *= (1.0f - drag);
}

void PhysicsObject::update(float dt) {
    if (!isStatic) {
        velocity += glm::vec3(0.0f, -gravity, 0.0f) * dt; // -0.98 is gravity
        position += velocity * dt;
    }
    collisions.clear();
}

void PhysicsEngine::addObject(std::shared_ptr<PhysicsObject> obj) {
    objects.push_back(obj);
}

void PhysicsEngine::simulate(float dt) {
    for (auto &obj : objects)
        obj->update(dt);

    handleCollisions();

    for (auto &obj : objects)
        obj->applySlowdown(0.05f); // friction/drag
}

bool checkSphereSphere(std::shared_ptr<PhysicsObject> a, std::shared_ptr<PhysicsObject> b) {
    float rSum = a->size.x + b->size.x;
    return pow(glm::distance(a->position, b->position), 2) <= rSum * rSum;
}

bool checkAABB(std::shared_ptr<PhysicsObject> a, std::shared_ptr<PhysicsObject> b) {
    return glm::all(glm::lessThanEqual(glm::abs(a->position - b->position), (a->size + b->size) * 0.5f));
}

bool checkCylinderCylinder(std::shared_ptr<PhysicsObject> a, std::shared_ptr<PhysicsObject> b) {
    float distXY = glm::length(glm::vec2(a->position.x - b->position.x, a->position.y - b->position.y));
    float rSum = a->size.x + b->size.x;
    bool zOverlap = std::abs(a->position.z - b->position.z) <= (a->size.z + b->size.z) * 0.5f;
    return distXY <= rSum && zOverlap;
}

bool checkSphereAABB(std::shared_ptr<PhysicsObject> sphere, std::shared_ptr<PhysicsObject> box) {
    glm::vec3 boxMin = box->position - box->size * 0.5f;
    glm::vec3 boxMax = box->position + box->size * 0.5f;

    glm::vec3 closest = glm::clamp(sphere->position, boxMin, boxMax);
    float distSq = pow(glm::distance(sphere->position, closest), 2);
    float r = sphere->size.x;
    return distSq <= r * r;
}

bool checkSphereCylinder(std::shared_ptr<PhysicsObject> sphere, std::shared_ptr<PhysicsObject> cylinder) {
    glm::vec2 sphereXY = glm::vec2(sphere->position);
    glm::vec2 cylXY = glm::vec2(cylinder->position);
    float distXY = glm::distance(sphereXY, cylXY);

    float rSum = sphere->size.x + cylinder->size.x;
    float zDist = std::abs(sphere->position.z - cylinder->position.z);
    float zOverlap = (sphere->size.z * 0.5f + cylinder->size.z * 0.5f);
    return distXY <= rSum && zDist <= zOverlap;
}

bool checkAABBCylinder(std::shared_ptr<PhysicsObject> box, std::shared_ptr<PhysicsObject> cylinder) {
    glm::vec3 boxMin = box->position - box->size * 0.5f;
    glm::vec3 boxMax = box->position + box->size * 0.5f;

    glm::vec2 cylXY = glm::vec2(cylinder->position);
    glm::vec2 closestXY = glm::clamp(cylXY, glm::vec2(boxMin), glm::vec2(boxMax));
    float distXY = glm::distance(cylXY, closestXY);

    float radius = cylinder->size.x;
    float zDist = std::abs(cylinder->position.z - box->position.z);
    float zOverlap = (box->size.z + cylinder->size.z) * 0.5f;
    return distXY <= radius && zDist <= zOverlap;
}

bool checkCollision(std::shared_ptr<PhysicsObject> a, std::shared_ptr<PhysicsObject> b) {
    ShapeType sa = a->shape;
    ShapeType sb = b->shape;
    if (sa > sb) std::swap(a, b), std::swap(sa, sb);

    if (!checkAABB(a, b)) return false;

    if (sa == ShapeType::Sphere && sb == ShapeType::Sphere)
        return checkSphereSphere(a, b);
    if (sa == ShapeType::Sphere && sb == ShapeType::Rectangle)
        return checkSphereAABB(a, b);
    if (sa == ShapeType::Sphere && sb == ShapeType::Cylinder)
        return checkSphereCylinder(a, b);
    if (sa == ShapeType::Rectangle && sb == ShapeType::Rectangle)
        return checkAABB(a, b);
    if (sa == ShapeType::Rectangle && sb == ShapeType::Cylinder)
        return checkAABBCylinder(a, b);
    if (sa == ShapeType::Cylinder && sb == ShapeType::Cylinder)
        return checkCylinderCylinder(a, b);

    return false;
}


void PhysicsEngine::handleCollisions() {
    std::unordered_map<GridCoord, std::vector<std::shared_ptr<PhysicsObject>>> spatialMap;

    // Build spatial grid
    for (auto &obj : objects) {
        GridCoord g = toGrid(obj->position);
        spatialMap[g].push_back(obj);
    }

    // Check collisions within each grid cell
    for (auto &[grid, cellObjects] : spatialMap) {
        for (size_t i = 0; i < cellObjects.size(); ++i) {
            for (size_t j = i + 1; j < cellObjects.size(); ++j) {
                auto a = cellObjects[i], b = cellObjects[j];
                if (!checkCollision(a, b)) continue;

                glm::vec3 contactPoint = (a->position + b->position) * 0.5f;
                a->collisions.push_back({b, contactPoint});
                b->collisions.push_back({a, contactPoint});

                glm::vec3 normal = glm::normalize(b->position - a->position);
                glm::vec3 relVel = b->velocity - a->velocity;
                float velAlongNormal = glm::dot(relVel, normal);

                if (velAlongNormal < 0) {
                    if (!a->isStatic && a->velocity != glm::vec3(0, 0, 0)) {
                        glm::vec3 stopComponent = glm::dot(a->velocity, normal) * normal;
                        a->velocity -= stopComponent;
                    }
                    if (!b->isStatic && b->velocity != glm::vec3(0, 0, 0)) {
                        glm::vec3 stopComponent = glm::dot(b->velocity, normal) * normal;
                        b->velocity -= stopComponent;
                    }
                }
            }
        }
    }
}
