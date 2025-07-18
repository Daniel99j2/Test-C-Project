#include "World.h"

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/common.hpp>
#include <cmath>
#include <unordered_map>
#include <memory>

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

static bool checkAABBAABB(const glm::vec3& aMin, const glm::vec3& aMax,
                          const glm::vec3& bMin, const glm::vec3& bMax) {
    return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
           (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
           (aMin.z <= bMax.z && aMax.z >= bMin.z);
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

                for (const auto& partA : *a->collisionParts) {
                    for (const auto& partB : *b->collisionParts) {
                        glm::vec3 aMin = a->position + partA.start;
                        glm::vec3 aMax = a->position + partA.end;
                        glm::vec3 bMin = b->position + partB.start;
                        glm::vec3 bMax = b->position + partB.end;

                        if (checkAABBAABB(aMin, aMax, bMin, bMax)) {
                            anyCollision = true;
                            break;
                        }
                    }
                    if (anyCollision) break;
                }

                if (!anyCollision) continue;

                a->collisions.push_back({ b, contactPoint });
                b->collisions.push_back({ a, contactPoint });

                glm::vec3 normal = glm::normalize(b->position - a->position);
                glm::vec3 relVel = b->velocity - a->velocity;
                float velAlong = glm::dot(relVel, normal);

                if (velAlong < 0) {
                    if (!a->isStatic && a->velocity != glm::vec3(0))
                        a->velocity -= glm::dot(a->velocity, normal) * normal;
                    if (!b->isStatic && b->velocity != glm::vec3(0))
                        b->velocity -= glm::dot(b->velocity, normal) * normal;
                }
            }
        }
    }
}
