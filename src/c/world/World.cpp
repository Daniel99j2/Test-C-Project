#include "World.h"

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/common.hpp>
#include <cmath>

#include "../util/Profiler.h"

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
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, RenderUtil::getAtlas());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, RenderUtil::getMERAtlas());

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
static bool checkAABB(std::shared_ptr<CollisionPart> a, const glm::vec3& posA,
                      std::shared_ptr<CollisionPart> b, const glm::vec3& posB) {
    return glm::all(glm::lessThanEqual(glm::abs(posA - posB), (a->size + b->size) * 0.5f));
}

static bool checkSphereSphere(std::shared_ptr<CollisionPart> a, const glm::vec3& posA,
                              std::shared_ptr<CollisionPart> b, const glm::vec3& posB) {
    float rSum = a->size.x + b->size.x;
    return glm::distance(posA, posB) <= rSum;
}

static bool checkSphereAABB(std::shared_ptr<CollisionPart> sphere, const glm::vec3& spherePos,
                            std::shared_ptr<CollisionPart> box, const glm::vec3& boxPos) {
    glm::vec3 boxMin = boxPos - box->size * 0.5f;
    glm::vec3 boxMax = boxPos + box->size * 0.5f;
    glm::vec3 closest = glm::clamp(spherePos, boxMin, boxMax);
    return glm::distance(spherePos, closest) <= sphere->size.x;
}

static bool checkSphereCylinder(std::shared_ptr<CollisionPart> sphere, const glm::vec3& spherePos,
                                std::shared_ptr<CollisionPart> cylinder, const glm::vec3& cylinderPos) {
    glm::vec2 s = glm::vec2(spherePos), c = glm::vec2(cylinderPos);
    float rSum = sphere->size.x + cylinder->size.x;
    float zDist = std::abs(spherePos.z - cylinderPos.z);
    float zOverlap = (sphere->size.z + cylinder->size.z) * 0.5f;
    return glm::distance(s, c) <= rSum && zDist <= zOverlap;
}

static bool checkCylinderCylinder(std::shared_ptr<CollisionPart> a, const glm::vec3& posA,
                                  std::shared_ptr<CollisionPart> b, const glm::vec3& posB) {
    glm::vec2 A = glm::vec2(posA), B = glm::vec2(posB);
    float rSum = a->size.x + b->size.x;
    float zDist = std::abs(posA.z - posB.z);
    return glm::distance(A, B) <= rSum && zDist <= (a->size.z + b->size.z) * 0.5f;
}

static bool checkAABBCylinder(std::shared_ptr<CollisionPart> box, const glm::vec3& boxPos,
                              std::shared_ptr<CollisionPart> cylinder, const glm::vec3& cylPos) {
    glm::vec3 min = boxPos - box->size * 0.5f;
    glm::vec3 max = boxPos + box->size * 0.5f;
    glm::vec2 cylXY = glm::vec2(cylPos);
    glm::vec2 closest = glm::clamp(cylXY, glm::vec2(min), glm::vec2(max));
    float distXY = glm::distance(closest, cylXY);
    float zDist = std::abs(cylPos.z - boxPos.z);
    return distXY <= cylinder->size.x && zDist <= (box->size.z + cylinder->size.z) * 0.5f;
}

static bool checkCollision(std::shared_ptr<CollisionPart> aIn, const glm::vec3& posAIn,
                           std::shared_ptr<CollisionPart> bIn, const glm::vec3& posBIn) {
    auto a = aIn;
    auto b = bIn;
    glm::vec3 posA = posAIn;
    glm::vec3 posB = posBIn;

    auto sa = a->shape;
    auto sb = b->shape;

    if (sa > sb) {
        std::swap(a, b);
        std::swap(posA, posB);
        std::swap(sa, sb);
    }

    if (!checkAABB(a, posA, b, posB)) return false;

    if (sa == ShapeType::Sphere && sb == ShapeType::Sphere)
        return checkSphereSphere(a, posA, b, posB);

    if (sa == ShapeType::Sphere && sb == ShapeType::Rectangle)
        return checkSphereAABB(a, posA, b, posB);

    if (sa == ShapeType::Sphere && sb == ShapeType::Cylinder)
        return checkSphereCylinder(a, posA, b, posB);

    if (sa == ShapeType::Rectangle && sb == ShapeType::Rectangle)
        return true;

    if (sa == ShapeType::Rectangle && sb == ShapeType::Cylinder)
        return checkAABBCylinder(a, posA, b, posB);

    if (sa == ShapeType::Cylinder && sb == ShapeType::Cylinder)
        return checkCylinderCylinder(a, posA, b, posB);

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

                bool anyCollision = false;
                glm::vec3 contactPoint = (a->position + b->position) * 0.5f;

                // Loop through all CollisionParts of both objects
                for (const auto& partA : *a->collisionParts) {
                    for (const auto& partB : *b->collisionParts) {
                        auto partAShared = std::make_shared<CollisionPart>(partA);
                        auto partBShared = std::make_shared<CollisionPart>(partB);

                        glm::vec3 posA = a->position + partA.position;
                        glm::vec3 posB = b->position + partB.position;

                        if (checkCollision(partAShared, posA, partBShared, posB)) {
                            anyCollision = true;
                            break;
                        }
                    }
                    if (anyCollision) break;
                }

                if (!anyCollision) continue;

                // Register collisions
                a->collisions.push_back({ b, contactPoint });
                b->collisions.push_back({ a, contactPoint });

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
