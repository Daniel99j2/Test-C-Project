#include "World.h"

#include "libs/glm/glm.hpp"
#include "libs/glm/geometric.hpp"
#include "libs/glm/common.hpp"
#include <cmath>

#include "src/c/util/Profiler.h"

constexpr float GRID_SIZE = 5.0f;

struct GridCoord {
    int x, y, z;
    bool operator==(const GridCoord& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {
    template <>
    struct hash<GridCoord> {
        std::size_t operator()(const GridCoord& g) const {
            return ((hash<int>()(g.x) ^ (hash<int>()(g.y) << 1)) >> 1) ^ (hash<int>()(g.z) << 1);
        }
    };
}

static GridCoord toGrid(const glm::vec3& pos) {
    return {
        static_cast<int>(std::floor(pos.x / GRID_SIZE)),
        static_cast<int>(std::floor(pos.y / GRID_SIZE)),
        static_cast<int>(std::floor(pos.z / GRID_SIZE))
    };
}

void World::drawWorld(float deltaTime) const {
    for (auto& object : gameObjects) {
        object->draw(deltaTime);
    }
}

void World::tick(float deltaTime) {
    Profiler::beginSection("Physics");
    simulatePhysics(deltaTime);
    Profiler::endSection("Physics");
    for (auto& object : gameObjects) {
        object->baseTick();
    }
}

void World::addObject(std::shared_ptr<GameObject> object) {
    object->id = currentId++;
    gameObjects.push_back(object);
}

void World::simulatePhysics(float dt) {
    for (auto& obj : gameObjects)
        obj->update(dt);

    handleCollisions();

    for (auto& obj : gameObjects)
        obj->applySlowdown(0.05f);
}

// Collision checkers
static bool checkAABB(std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b) {
    return glm::all(glm::lessThanEqual(glm::abs(a->position - b->position), (a->size + b->size) * 0.5f));
}

static bool checkSphereSphere(std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b) {
    float rSum = a->size.x + b->size.x;
    return glm::distance(a->position, b->position) <= rSum;
}

static bool checkSphereAABB(std::shared_ptr<GameObject> sphere, std::shared_ptr<GameObject> box) {
    glm::vec3 boxMin = box->position - box->size * 0.5f;
    glm::vec3 boxMax = box->position + box->size * 0.5f;
    glm::vec3 closest = glm::clamp(sphere->position, boxMin, boxMax);
    return glm::distance(sphere->position, closest) <= (sphere->size.x);
}

static bool checkSphereCylinder(std::shared_ptr<GameObject> sphere, std::shared_ptr<GameObject> cylinder) {
    glm::vec2 s = glm::vec2(sphere->position), c = glm::vec2(cylinder->position);
    float rSum = sphere->size.x + cylinder->size.x;
    float zDist = std::abs(sphere->position.z - cylinder->position.z);
    float zOverlap = (sphere->size.z + cylinder->size.z) * 0.5f;
    return glm::distance(s, c) <= rSum && zDist <= zOverlap;
}

static bool checkCylinderCylinder(std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b) {
    glm::vec2 A = glm::vec2(a->position), B = glm::vec2(b->position);
    float rSum = a->size.x + b->size.x;
    float zDist = std::abs(a->position.z - b->position.z);
    return glm::distance(A, B) <= rSum && zDist <= (a->size.z + b->size.z) * 0.5f;
}

static bool checkAABBCylinder(std::shared_ptr<GameObject> box, std::shared_ptr<GameObject> cylinder) {
    glm::vec3 min = box->position - box->size * 0.5f;
    glm::vec3 max = box->position + box->size * 0.5f;
    glm::vec2 cylXY = glm::vec2(cylinder->position);
    glm::vec2 closest = glm::clamp(cylXY, glm::vec2(min), glm::vec2(max));
    float distXY = glm::distance(closest, cylXY);
    float zDist = std::abs(cylinder->position.z - box->position.z);
    return distXY <= cylinder->size.x && zDist <= (box->size.z + cylinder->size.z) * 0.5f;
}

static bool checkCollision(std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b) {
    auto sa = a->shape, sb = b->shape;
    if (sa > sb) std::swap(a, b), std::swap(sa, sb);
    if (!checkAABB(a, b)) return false;

    if (sa == ShapeType::Sphere && sb == ShapeType::Sphere) return checkSphereSphere(a, b);
    if (sa == ShapeType::Sphere && sb == ShapeType::Rectangle) return checkSphereAABB(a, b);
    if (sa == ShapeType::Sphere && sb == ShapeType::Cylinder) return checkSphereCylinder(a, b);
    if (sa == ShapeType::Rectangle && sb == ShapeType::Rectangle) return true;
    if (sa == ShapeType::Rectangle && sb == ShapeType::Cylinder) return checkAABBCylinder(a, b);
    if (sa == ShapeType::Cylinder && sb == ShapeType::Cylinder) return checkCylinderCylinder(a, b);

    return false;
}

void World::handleCollisions() const {
    std::unordered_map<GridCoord, std::vector<std::shared_ptr<GameObject>>> spatial;

    for (auto& obj : gameObjects)
        spatial[toGrid(obj->position)].push_back(obj);

    for (auto& [cell, objs] : spatial) {
        for (size_t i = 0; i < objs.size(); ++i) {
            for (size_t j = i + 1; j < objs.size(); ++j) {
                auto a = objs[i], b = objs[j];
                if (!checkCollision(a, b)) continue;

                glm::vec3 contact = (a->position + b->position) * 0.5f;
                a->collisions.push_back({b, contact});
                b->collisions.push_back({a, contact});

                glm::vec3 normal = glm::normalize(b->position - a->position);
                glm::vec3 relVel = b->velocity - a->velocity;
                float velAlong = glm::dot(relVel, normal);

                if (velAlong < 0) {
                    if (!a->isStatic && a->velocity != glm::vec3(0)) {
                        a->velocity -= glm::dot(a->velocity, normal) * normal;
                    }
                    if (!b->isStatic && b->velocity != glm::vec3(0)) {
                        b->velocity -= glm::dot(b->velocity, normal) * normal;
                    }
                }
            }
        }
    }
}
